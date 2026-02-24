#include "auto_run_on_startup.h"

#include <qapplication.h>
#include <qdir.h>
#include <qstring.h>
#include <qsettings.h>

static constexpr const char* RUN_KEY =
    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";

static QString getAppRegistryPath()
{
    QString appPath = QApplication::applicationFilePath();
    if (appPath.isEmpty())
        return QString();
    return "\"" + QDir::toNativeSeparators(appPath) + "\"";
}

bool isAutoRunOnStartUp()
{
    QSettings settings(RUN_KEY, QSettings::NativeFormat);

    QString appName = QApplication::applicationName();
    QString appPath = getAppRegistryPath();
    if (appPath.isEmpty())
        return false;

    return settings.contains(appName) && settings.value(appName) == appPath;
}

bool setAutoRunOnStartUp(bool enable)
{
    QSettings settings(RUN_KEY, QSettings::NativeFormat);

    QString appName = QApplication::applicationName();
    if (appName.isEmpty())
        return false;

    QString appPath = getAppRegistryPath();
    if (appPath.isEmpty())
        return false;

    enable ? settings.setValue(appName, appPath) : settings.remove(appName);

    settings.sync();
    return settings.status() == QSettings::NoError;
}
