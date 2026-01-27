#include "settings_dialog.h"

#include <qmessagebox.h>

#include <easy_translate.hpp>

#include "auto_run_on_startup.h"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent), settings_(loadSettings())
{
    ui.setupUi(this);

    connect(ui.languageComboBox, &QComboBox::currentIndexChanged, this, &SettingsDialog::onLanguageChanged);
    connect(ui.autoRunOnStartUpCb, &QCheckBox::toggled, this, &SettingsDialog::onAutoRunOnStartUpChanged);
    connect(ui.keepDialogOnErrorOccurredCb, &QCheckBox::toggled, this, &SettingsDialog::onKeepDialogOnErrorOccurredChanged);
    connect(ui.removeToTrashCb, &QCheckBox::toggled, this, &SettingsDialog::onRemoveToTrashChanged);
    connect(ui.symlinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsDialog::onSymlinkHotkeyChanged);
    connect(ui.hardlinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsDialog::onHardlinkHotkeyChanged);

    updateText();
}

void SettingsDialog::updateText()
{
    setWindowTitle(EASYTR("SettingsDialog.Title"));
    ui.languageText->setText(EASYTR("SettingsDialog.Text.Language"));
    ui.autoRunOnStartUpCb->setText(EASYTR("SettingsDialog.CheckBox.AutoRunOnStartUp"));
    ui.keepDialogOnErrorOccurredCb->setText(EASYTR("SettingsDialog.CheckBox.KeepDialogOnErrorOccurred"));
    ui.removeToTrashCb->setText(EASYTR("SettingsDialog.CheckBox.RemoveToTrash"));
    ui.symlinkHotkeyText->setText(EASYTR("SettingsDialog.Text.SymlinkHotkey"));
    ui.hardlinkHotkeyText->setText(EASYTR("SettingsDialog.Text.HardlinkHotkey"));
    ui.symlinkHotkeyInputer->setToolTip(EASYTR("SettingsDialog.KeyCombinationInputer.ToolTip"));
    ui.hardlinkHotkeyInputer->setToolTip(EASYTR("SettingsDialog.KeyCombinationInputer.ToolTip"));

    ui.languageComboBox->setItemText(0, EASYTR("SettingsDialog.ComboBox.ItemText.English"));
    ui.languageComboBox->setItemText(1, EASYTR("SettingsDialog.ComboBox.ItemText.Chinese"));
}

void SettingsDialog::onLanguageChanged(int index)
{
    Language lang = (Language) index;
    settings_.language = lang;
    saveSettings(settings_);
    if (!setLanguage(lang))
        qDebug() << QString("Failed to set language to %1").arg(languageStringId(lang));
}

void SettingsDialog::onAutoRunOnStartUpChanged(bool enable)
{
    settings_.autoRunOnStartUp = enable;
    saveSettings(settings_);
    if (isAutoRunOnStartUp() != enable)
    {
        if (!setAutoRunOnStartUp(enable))
            qDebug() << QString("Failed to %1 auto run on start up").arg(enable ? "set" : "unset");
    }
}

void SettingsDialog::onKeepDialogOnErrorOccurredChanged(bool enable)
{
    settings_.linkConfig.keepDialogOnErrorOccurred = enable;
    saveSettings(settings_);
}

void SettingsDialog::onRemoveToTrashChanged(bool enable)
{
    settings_.linkConfig.removeToTrash = enable;
    saveSettings(settings_);
}

void SettingsDialog::onSymlinkHotkeyChanged(QKeyCombination qkc)
{
    if (qkc == ui.hardlinkHotkeyInputer->keyCombination())
    {
        // Roll back
        auto qks = QKeySequence::fromString(settings_.symlinkHotkey.toString().c_str());
        ui.symlinkHotkeyInputer->setKeyCombination(qks);

        alertSameHotkey();
    }

    settings_.symlinkHotkey = gbhk::KeyCombination::fromString(QKeySequence(qkc).toString().toStdString());
    saveSettings(settings_);
    emit symlinkHotkeyChanged(settings_.symlinkHotkey);
}

void SettingsDialog::onHardlinkHotkeyChanged(QKeyCombination qkc)
{
    if (qkc == ui.symlinkHotkeyInputer->keyCombination())
    {
        // Roll back
        auto qks = QKeySequence::fromString(settings_.hardlinkHotkey.toString().c_str());
        ui.hardlinkHotkeyInputer->setKeyCombination(qks);

        alertSameHotkey();
    }

    settings_.hardlinkHotkey = gbhk::KeyCombination::fromString(QKeySequence(qkc).toString().toStdString());
    saveSettings(settings_);
    emit hardlinkHotkeyChanged(settings_.hardlinkHotkey);
}

void SettingsDialog::alertSameHotkey()
{
    QMessageBox msgBox(
        QMessageBox::Warning,
        EASYTR("SettingsDialog.MessageBox.AlertSameHotkey.Title"),
        EASYTR("SettingsDialog.MessageBox.AlertSameHotkey.Text"),
        QMessageBox::Ok,
        this
    );
    msgBox.exec();
}
