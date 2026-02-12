#include "auto_run_on_startup.h"

#include <cstdlib>  // system

#include <unistd.h> // getuid

#include <qcoreapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

#include "config.h"

static QString buildPlistContent(const QString& label, const QString& exePath)
{
    static QString content = QString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
        "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "    <key>Label</key>\n"
        "    <string>%1</string>\n"
        "    <key>ProgramArguments</key>\n"
        "    <array>\n"
        "        <string>open</string>\n"
        "        <string>-a</string>\n"
        "        <string>%2</string>\n"
        "    </array>\n"
        "    <key>RunAtLoad</key>\n"
        "    <true/>\n"
        "</dict>\n"
        "</plist>\n"
    ).arg(label, exePath);
    return content;
}

static const QString launchAgentPlistPath =
    QString("%1/Library/LaunchAgents/%2.%3.plist").arg(QDir::homePath(), APP_ORGANIZATION_DOMAIN, APP_TITLE);

bool isAutoRunOnStartUp()
{
    QFileInfo fi(launchAgentPlistPath);
    return fi.exists();
}

bool setAutoRunOnStartUp(bool enable)
{
    auto exePath = QCoreApplication::applicationFilePath();
    if (exePath.isEmpty())
        return false;

    uid_t uid = getuid();
    QString guiDomain = QString("gui/%1").arg(uid);

    if (enable)
    {
        QDir dir(QDir::homePath() + "/Library/LaunchAgents");
        if (!dir.exists() && !dir.mkpath("."))
            return false;

        QFile file(launchAgentPlistPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return false;

        QTextStream ts(&file);
        ts << buildPlistContent(APP_ORGANIZATION_DOMAIN, exePath);
        file.close();

        QString cmd = QString("launchctl bootstrap %1 \"%2\"").arg(guiDomain, launchAgentPlistPath);
        int rc = std::system(cmd.toUtf8().constData());

        return rc == 0;
    }
    else
    {
        QString cmd = QString("launchctl bootout %1 \"%2\"").arg(guiDomain, launchAgentPlistPath);
        std::system(cmd.toUtf8().constData());

        if (QFile::exists(launchAgentPlistPath))
            return QFile::remove(launchAgentPlistPath);
        return true;
    }
}
