#include "auto_run_on_startup.h"

#include <cstdlib>  // system

#include <unistd.h> // getuid

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

#include "config.h"
#include "utils/file_io.h"

static QString buildPlistContent(const QString& label, const QString& appPath)
{
    QString content =
        readAllFromFile(APP_RESOURCE_DIRPATH + "/scripts/auto_run_on_startup_format.plist").arg(
            label, appPath);
    return content;
}

static const QString plistName = QString("%1.%2").arg(APP_ORGANIZATION_DOMAIN, APP_TITLE);
static const QString launchAgentPlistPath =
    QString("%1/Library/LaunchAgents/%2.plist").arg(QDir::homePath(), plistName);

bool isAutoRunOnStartUp()
{
    QFileInfo fi(launchAgentPlistPath);
    return fi.exists();
}

bool setAutoRunOnStartUp(bool enable)
{
    auto appPath = QApplication::applicationFilePath();
    if (appPath.isEmpty())
        return false;

    uid_t uid = getuid();
    QString guiDomain = QString("gui/%1").arg(uid);

    if (enable)
    {
        QDir launchAgentPlistDir = QDir(QFileInfo(launchAgentPlistPath).absolutePath());
        if (!launchAgentPlistDir.exists())
        {
            if (launchAgentPlistDir.mkpath("."))
                return false;
        }

        QFile file(launchAgentPlistPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return false;

        QTextStream ts(&file);
        ts << buildPlistContent(plistName, appPath);
        file.close();

        QString cmd = QString("launchctl bootstrap %1 \"%2\"").arg(guiDomain, launchAgentPlistPath);
        int rc = system(cmd.toUtf8().constData());

        return rc == 0;
    }
    else
    {
        QString cmd = QString("launchctl bootout %1 \"%2\"").arg(guiDomain, launchAgentPlistPath);
        system(cmd.toUtf8().constData());

        if (QFile::exists(launchAgentPlistPath))
            return QFile::remove(launchAgentPlistPath);
        return true;
    }
}
