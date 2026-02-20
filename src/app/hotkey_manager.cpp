#include "hotkey_manager.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qdir.h>
#include <qmimedata.h>

#include "filelink/controller.h"
#include "platforms/filemgr_dir_getter.h"

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent),
    ghm_(gbhk::HookGlobalHotkeyManager::getInstance())
{
    int rc = ghm_.run();
    if (rc != gbhk::RC_SUCCESS)
        qCritical() << "Failed to run the Global Hotkey Manager, error message: "
                    << gbhk::getReturnCodeMessage(rc).c_str();
    connect(this, &HotkeyManager::shouldLinks, this, &HotkeyManager::links);
}

HotkeyManager::~HotkeyManager()
{
    int rc = ghm_.stop();
    if (rc != gbhk::RC_SUCCESS)
        qCritical() << "Failed to stop the Global Hotkey Manager, error message: "
                    << gbhk::getReturnCodeMessage(rc).c_str();
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
                    qCritical() << QString("Failed to replace hotkey from '%1' to '%2', error message: %3").arg(
                        oldHotkey.toString().c_str(),
                        newHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str()).toUtf8().constData();
            }
            else
            {
                int rc = ghm_.unregisterHotkey(oldHotkey);
                if (rc != gbhk::RC_SUCCESS)
                    qCritical() << QString("Failed to unregister hotkey '%1', error message: %2").arg(
                        oldHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str()).toUtf8().constData();
            }
        }
        else
        {
            if (newHotkey.isValid())
            {
                int rc = ghm_.registerHotkey(newHotkey, [=]() { emit shouldLinks(linkType); });
                if (rc != gbhk::RC_SUCCESS)
                    qCritical() << QString("Failed to register hotkey '%1', error message: %2").arg(
                        newHotkey.toString().c_str(),
                        gbhk::getReturnCodeMessage(rc).c_str()).toUtf8().constData();
            }
        }
    };

    if (settings.symlinkHotkey != settings_.symlinkHotkey)
        hotkeyHandler(LT_SYMLINK);
    if (settings.hardlinkHotkey != settings_.hardlinkHotkey)
        hotkeyHandler(LT_HARDLINK);

    settings_ = settings;
}

static inline QString removeLastSeparator(QString path)
{
    while (path.endsWith('/') || path.endsWith('\\'))
        path.chop(1);
    return path;
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
            qWarning() << e.what();
            return;
        }

        targetDir = QDir(targetDir).canonicalPath();
        // 任务结束后，其会自动释放。
        auto controller = new FileLinkController(linkType, sourcePaths, targetDir, settings_.linkConfig, this);
        controller->start();
    }
}
