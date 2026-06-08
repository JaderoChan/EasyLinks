#include "hotkey_manager.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmimedata.h>

#include "filelink/controller.h"
#include "platforms/filemgr_dir_getter.h"
#include "utils/logging.h"

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent),
    ghm_(gbhk::HookGlobalHotkeyManager::getInstance())
{
    int rc = ghm_.run();
    if (rc != gbhk::RC_SUCCESS)
        debugOut(qCritical(), "[Hotkey Manager] Failed to run the Global Hotkey Manager. Error message: %1.",
            gbhk::getReturnCodeMessage(rc).c_str());
    connect(this, &HotkeyManager::shouldLinks, this, &HotkeyManager::links);
    connect(this, &HotkeyManager::shouldPatternLink, this, &HotkeyManager::patternLink);
}

HotkeyManager::~HotkeyManager()
{
    int rc = ghm_.stop();
    if (rc != gbhk::RC_SUCCESS)
        debugOut(qCritical(), "[Hotkey Manager] Failed to stop the Global Hotkey Manager. Error message: %1.",
            gbhk::getReturnCodeMessage(rc).c_str());
}

void HotkeyManager::setSettings(const Settings& settings)
{
    auto hotkeyHandler = [=](LinkType linkType)
    {
        auto oldHotkey = (linkType == LT_SYMLINK ? settings_.symlinkHotkey : settings_.hardlinkHotkey);
        auto newHotkey = (linkType == LT_SYMLINK ? settings.symlinkHotkey : settings.hardlinkHotkey);

        if (ghm_.isHotkeyRegistered(oldHotkey))
        {
            if (newHotkey.isValid())
            {
                int rc = ghm_.replaceHotkey(oldHotkey, newHotkey);
                if (rc != gbhk::RC_SUCCESS)
                    debugOut(qCritical(), "[Hotkey Manager] Failed to replace hotkey from '%1' to '%2'. Error message: %3.",
                        oldHotkey.toString().c_str(),
                        newHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str());
            }
            else
            {
                int rc = ghm_.unregisterHotkey(oldHotkey);
                if (rc != gbhk::RC_SUCCESS)
                    debugOut(qCritical(), "[Hotkey Manager] Failed to unregister hotkey '%1'. Error message: %2.",
                        oldHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str());
            }
        }
        else
        {
            if (newHotkey.isValid())
            {
                int rc = ghm_.registerHotkey(newHotkey, [=]() { emit shouldLinks(linkType); });
                if (rc != gbhk::RC_SUCCESS)
                    debugOut(qCritical(), "[Hotkey Manager] Failed to register hotkey '%1'. Error message: %2.",
                        newHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str());
            }
        }
    };

    if (settings.symlinkHotkey != settings_.symlinkHotkey)
        hotkeyHandler(LT_SYMLINK);
    if (settings.hardlinkHotkey != settings_.hardlinkHotkey)
        hotkeyHandler(LT_HARDLINK);

    auto oldPatternLinkHotkey = settings_.patternLinkHotkey;
    auto newPatternLinkHotkey = settings.patternLinkHotkey;
    if (newPatternLinkHotkey != oldPatternLinkHotkey)
    {
        if (ghm_.isHotkeyRegistered(oldPatternLinkHotkey))
        {
            if (newPatternLinkHotkey.isValid())
            {
                int rc = ghm_.replaceHotkey(oldPatternLinkHotkey, newPatternLinkHotkey);
                if (rc != gbhk::RC_SUCCESS)
                    debugOut(qCritical(), "[Hotkey Manager] Failed to replace hotkey from '%1' to '%2'. Error message: %3.",
                        oldPatternLinkHotkey.toString().c_str(),
                        newPatternLinkHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str());
            }
            else
            {
                int rc = ghm_.unregisterHotkey(oldPatternLinkHotkey);
                if (rc != gbhk::RC_SUCCESS)
                    debugOut(qCritical(), "[Hotkey Manager] Failed to unregister hotkey '%1'. Error message: %2.",
                        oldPatternLinkHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str());
            }
        }
        else if (newPatternLinkHotkey.isValid())
        {
            int rc = ghm_.registerHotkey(newPatternLinkHotkey, [=]() { emit shouldPatternLink(); });
            if (rc != gbhk::RC_SUCCESS)
                debugOut(qCritical(), "[Hotkey Manager] Failed to register hotkey '%1'. Error message: %2.",
                    newPatternLinkHotkey.toString().c_str(),
                    gbhk::getReturnCodeMessage(rc).c_str());
        }
    }

    settings_ = settings;
}

static inline QString removeLastSeparator(QString path)
{
    while (path.endsWith('/') || path.endsWith('\\'))
        path.chop(1);
    return path;
}

static inline QString normalizeDirPath(const QString& dir)
{
    QFileInfo fi(dir);
    QString normalized = fi.canonicalFilePath();
    if (normalized.isEmpty())
        normalized = fi.absoluteFilePath();

    normalized = removeLastSeparator(QDir::cleanPath(normalized));
    return normalized;
}

void HotkeyManager::links(LinkType linkType)
{
    auto data = qApp->clipboard()->mimeData();
    if (data->hasUrls())
    {
        QStringList sourcePaths;
        for (const auto& url : data->urls())
            sourcePaths.append(removeLastSeparator(url.toLocalFile()));

        QString targetDir;
        try { targetDir = getFocusedFileManagerDir(); }
        catch (const std::exception& e)
        {
            debugOut(qWarning(),
                "[Hotkey Triggered] Failed to get the focused file manager directory. Error message: %1.",
                e.what());
            return;
        }

        targetDir = normalizeDirPath(targetDir);
        if (!QFileInfo(targetDir).isDir())
            return;
        // 任务结束后，其会自动释放。
        auto controller = new FileLinkController(linkType, sourcePaths, targetDir, settings_.linkConfig, this);
        connect(controller, &FileLinkController::linkFinished, this,
        [=](LinkType lt, const QString& dir, const LinkStats& stats) { emit linkCompleted(lt, dir, stats); });
        controller->start();
    }
}

void HotkeyManager::patternLink()
{
    QString targetDir;
    try { targetDir = getFocusedFileManagerDir(); }
    catch (const std::exception& e)
    {
        debugOut(qWarning(),
            "[Pattern Link Triggered] Failed to get the focused file manager directory. Error message: %1.",
            e.what());
        return;
    }

    targetDir = normalizeDirPath(targetDir);
    if (!QFileInfo(targetDir).isDir())
        return;

    emit patternLinkTriggered(targetDir);
}
