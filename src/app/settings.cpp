#include "settings.h"

#include <qsettings.h>

#include "config.h"
#include "filelink/rename_pattern.h"

#define READ_KC(settings, key, defaultValue) \
gbhk::KeyCombination(settings.value(key, defaultValue).toString().toStdString())

Settings loadSettings()
{
    Settings settings;

    QSettings qsettings(QSettings::NativeFormat, QSettings::UserScope, APP_ORGANIZATION, APP_TITLE);
    settings.language = qsettings.value("Language", currentSystemLang()).value<Language>();
    settings.autoRunOnStartUp = qsettings.value("AutoRunOnStartUp", false).toBool();
    settings.symlinkHotkey = READ_KC(qsettings, "SymlinkHotkey", "Ctrl+S");
    settings.hardlinkHotkey = READ_KC(qsettings, "HardlinkHotkey", "Ctrl+H");

    qsettings.beginGroup("LinkConfig");
    settings.linkConfig.keepDialogOnErrorOccurred = qsettings.value("KeepDialogOnErrorOccurred", true).toBool();
    settings.linkConfig.removeToTrash = qsettings.value("RemoveToTrash", false).toBool();
    settings.linkConfig.renamePattern = qsettings.value("RenamePattern", DEFAULT_RENAME_PATTERN).toString();
    if (!isLegalRenamePattern(settings.linkConfig.renamePattern))
        settings.linkConfig.renamePattern = DEFAULT_RENAME_PATTERN;
    qsettings.endGroup();

    return settings;
}

void saveSettings(const Settings& settings)
{
    QSettings qsettings(QSettings::NativeFormat, QSettings::UserScope, APP_ORGANIZATION, APP_TITLE);
    qsettings.setValue("Language", settings.language);
    qsettings.setValue("AutoRunOnStartUp", settings.autoRunOnStartUp);
    qsettings.setValue("SymlinkHotkey", settings.symlinkHotkey.toString().c_str());
    qsettings.setValue("HardlinkHotkey", settings.hardlinkHotkey.toString().c_str());

    qsettings.beginGroup("LinkConfig");
    qsettings.setValue("KeepDialogOnErrorOccurred", settings.linkConfig.keepDialogOnErrorOccurred);
    qsettings.setValue("RemoveToTrash", settings.linkConfig.removeToTrash);
    qsettings.setValue("RenamePattern", settings.linkConfig.renamePattern);
    qsettings.endGroup();
}
