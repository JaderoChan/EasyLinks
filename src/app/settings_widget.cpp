#include "settings_widget.h"

#include <qmessagebox.h>
#include <qvalidator.h>

#include <easy_translate.hpp>

#include "filelink/rename_pattern.h"

SettingsWidget::SettingsWidget(const Settings& settings, QWidget* parent)
    : QWidget(parent), settings_(settings)
{
    ui.setupUi(this);

    ui.languageComboBox->setCurrentIndex((int) settings_.language);
    ui.autoRunOnStartUpCb->setChecked(settings_.autoRunOnStartUp);
    ui.keepDialogOnErrorOccurredCb->setChecked(settings_.linkConfig.keepDialogOnErrorOccurred);
    ui.removeToTrashCb->setChecked(settings_.linkConfig.removeToTrash);
    ui.renamePatternLe->setText(settings_.linkConfig.renamePattern);
    auto symlinkQks = QKeySequence::fromString(settings_.symlinkHotkey.toString().c_str());
    ui.symlinkHotkeyInputer->setKeyCombination(symlinkQks.isEmpty() ? QKeyCombination() : symlinkQks[0]);
    auto hardlinkQks = QKeySequence::fromString(settings_.hardlinkHotkey.toString().c_str());
    ui.hardlinkHotkeyInputer->setKeyCombination(hardlinkQks.isEmpty() ? QKeyCombination() : hardlinkQks[0]);

    auto validator = new QRegularExpressionValidator(this);
    // 要求Rename Pattern至少包含一个前面没有反斜杠的'@'字符和前面没有反斜杠的'#'字符。
    validator->setRegularExpression(QRegularExpression("^(?=.*(?<!\\)@)(?=.*(?<!\\)#)"));
    ui.renamePatternLe->setValidator(validator);

    connect(ui.languageComboBox, &QComboBox::currentIndexChanged, this, &SettingsWidget::onLanguageChanged);
    connect(ui.autoRunOnStartUpCb, &QCheckBox::toggled, this, &SettingsWidget::onAutoRunOnStartUpChanged);
    connect(ui.keepDialogOnErrorOccurredCb, &QCheckBox::toggled, this, &SettingsWidget::onKeepDialogOnErrorOccurredChanged);
    connect(ui.removeToTrashCb, &QCheckBox::toggled, this, &SettingsWidget::onRemoveToTrashChanged);
    connect(ui.renamePatternLe, &QLineEdit::textEdited, this, &SettingsWidget::onRenamePatternChanged);
    connect(ui.symlinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsWidget::onSymlinkHotkeyChanged);
    connect(ui.hardlinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsWidget::onHardlinkHotkeyChanged);

    updateText();
}

void SettingsWidget::showAndActivate()
{
    show();
    activateWindow();
    raise();
}

void SettingsWidget::updateText()
{
    setWindowTitle(EASYTR("SettingsWidget.WindowTitle"));

    ui.renamePatternGroupBox->setTitle(EASYTR("SettingsWidget.RenamePatternGroupBox.Title"));

    ui.languageText->setText(EASYTR("SettingsWidget.Text.Language"));
    ui.autoRunOnStartUpCb->setText(EASYTR("SettingsWidget.CheckBox.AutoRunOnStartUp"));
    ui.keepDialogOnErrorOccurredCb->setText(EASYTR("SettingsWidget.CheckBox.KeepDialogOnErrorOccurred"));
    ui.removeToTrashCb->setText(EASYTR("SettingsWidget.CheckBox.RemoveToTrash"));
    ui.renamePatternLe->setPlaceholderText(EASYTR("SettingsWidget.LineEdit.RenamePattern.Placeholder"));
    ui.renamePatternTips->setText(EASYTR("SettingsWidget.Text.RenamePatternTips"));
    ui.symlinkHotkeyText->setText(EASYTR("SettingsWidget.Text.SymlinkHotkey"));
    ui.hardlinkHotkeyText->setText(EASYTR("SettingsWidget.Text.HardlinkHotkey"));
    ui.symlinkHotkeyInputer->setToolTip(EASYTR("SettingsWidget.KeyCombinationInputer.ToolTip"));
    ui.hardlinkHotkeyInputer->setToolTip(EASYTR("SettingsWidget.KeyCombinationInputer.ToolTip"));

    ui.languageComboBox->setItemText(0, EASYTR("SettingsWidget.ComboBox.ItemText.English"));
    ui.languageComboBox->setItemText(1, EASYTR("SettingsWidget.ComboBox.ItemText.Chinese"));
}

void SettingsWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QWidget::changeEvent(event);
}

void SettingsWidget::onLanguageChanged(int index)
{
    settings_.language = (Language) index;
    emit settingsChanged(settings_);
}

void SettingsWidget::onAutoRunOnStartUpChanged(bool enable)
{
    settings_.autoRunOnStartUp = enable;
    emit settingsChanged(settings_);
}

void SettingsWidget::onKeepDialogOnErrorOccurredChanged(bool enable)
{
    settings_.linkConfig.keepDialogOnErrorOccurred = enable;
    emit settingsChanged(settings_);
}

void SettingsWidget::onRemoveToTrashChanged(bool enable)
{
    settings_.linkConfig.removeToTrash = enable;
    emit settingsChanged(settings_);
}

void SettingsWidget::onRenamePatternChanged(QString renamePattern)
{
    if (!isLegalRenamePattern(renamePattern))
    {
        ui.renamePatternLe->setText(settings_.linkConfig.renamePattern);
    }
    else
    {
        settings_.linkConfig.renamePattern = renamePattern;
        emit settingsChanged(settings_);
    }
}

void SettingsWidget::onSymlinkHotkeyChanged(QKeyCombination qkc)
{
    if (qkc == ui.hardlinkHotkeyInputer->keyCombination())
    {
        // Roll back
        auto qks = QKeySequence::fromString(settings_.symlinkHotkey.toString().c_str());
        ui.symlinkHotkeyInputer->setKeyCombination(qks);

        alertSameHotkey();
        return;
    }

    settings_.symlinkHotkey = gbhk::KeyCombination::fromString(QKeySequence(qkc).toString().toStdString());
    emit settingsChanged(settings_);
}

void SettingsWidget::onHardlinkHotkeyChanged(QKeyCombination qkc)
{
    if (qkc == ui.symlinkHotkeyInputer->keyCombination())
    {
        // Roll back
        auto qks = QKeySequence::fromString(settings_.hardlinkHotkey.toString().c_str());
        ui.hardlinkHotkeyInputer->setKeyCombination(qks);

        alertSameHotkey();
        return;
    }

    settings_.hardlinkHotkey = gbhk::KeyCombination::fromString(QKeySequence(qkc).toString().toStdString());
    emit settingsChanged(settings_);
}

void SettingsWidget::alertSameHotkey()
{
    QMessageBox msgBox(
        QMessageBox::Warning,
        EASYTR("SettingsWidget.MessageBox.AlertSameHotkey.Title"),
        EASYTR("SettingsWidget.MessageBox.AlertSameHotkey.Text"),
        QMessageBox::Ok,
        this
    );
    msgBox.exec();
}
