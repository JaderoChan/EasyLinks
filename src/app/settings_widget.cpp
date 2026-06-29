#include "settings_widget.h"

#include <qmessagebox.h>

#include <easy_translate.hpp>

#include "filelink/rename_pattern.h"

#ifdef Q_OS_MAC

namespace fix
{

static inline QKeyCombination swapCtrlMeta(const QKeyCombination& kc) noexcept
{
    Qt::KeyboardModifiers mod = kc.keyboardModifiers();

    // 在macOS上如果用户按下了Control/Meta键（其为`Qt::Modifier::META`/`Qt::Modifier::CTRL`），
    // 则映射至`Qt::Modifier::CTRL`/`Qt::Modifier::META`。
    if ((mod & Qt::Modifier::META) && !(mod & Qt::Modifier::CTRL))
        mod = Qt::KeyboardModifiers((mod & ~Qt::Modifier::META) | Qt::Modifier::CTRL);
    else if ((mod & Qt::Modifier::CTRL) && !(mod & Qt::Modifier::META))
        mod = Qt::KeyboardModifiers((mod & ~Qt::Modifier::CTRL) | Qt::Modifier::META);
    else
        return kc;

    return QKeyCombination(mod, kc.key());
};

} // namespace fix

#endif // Q_OS_MAC

static QKeyCombination gbhkKcToQtKc(const gbhk::KeyCombination& kc)
{
    auto qks = QKeySequence::fromString(kc.toString().c_str());
    if (qks.count() == 0)
        return QKeyCombination();
#ifdef Q_OS_MAC
    // macOS下交换Ctrl与Meta修饰键。
    return fix::swapCtrlMeta(qks[0]);
#else
    return qks[0];
#endif // Q_OS_MAC
}

static gbhk::KeyCombination qtKcToGbhkKc(const QKeyCombination& qkc)
{
#ifdef Q_OS_MAC
    QString kcStr = QKeySequence(fix::swapCtrlMeta(qkc)).toString();
#else
    QString kcStr = QKeySequence(qkc).toString();
#endif // Q_OS_MAC
    return gbhk::KeyCombination::fromString(kcStr.toStdString());
}

SettingsWidget::SettingsWidget(const Settings& settings, QWidget* parent)
    : QWidget(parent), settings_(settings)
{
    ui.setupUi(this);

    ui.languageComboBox->setCurrentIndex((int) settings_.language);
    ui.autoRunOnStartUpCb->setChecked(settings_.autoRunOnStartUp);
    ui.keepDialogOnErrorOccurredCb->setChecked(settings_.linkConfig.keepDialogOnErrorOccurred);
    ui.removeToTrashCb->setChecked(settings_.linkConfig.removeToTrash);
    ui.patternLinkModeComboBox->setCurrentIndex(settings_.patterns == SUPERFICIAL_PATTERNS ? 0 : 1);
    ui.renamePatternLe->setText(settings_.linkConfig.renamePattern);
    ui.symlinkHotkeyInputer->setKeyCombination(gbhkKcToQtKc(settings_.symlinkHotkey));
    ui.hardlinkHotkeyInputer->setKeyCombination(gbhkKcToQtKc(settings_.hardlinkHotkey));
    ui.patternLinkHotkeyInputer->setKeyCombination(gbhkKcToQtKc(settings_.patternLinkHotkey));

    ui.renamePatternLe->installEventFilter(this);

    connect(ui.languageComboBox, &QComboBox::currentIndexChanged, this, &SettingsWidget::onLanguageChanged);
    connect(ui.autoRunOnStartUpCb, &QCheckBox::toggled, this, &SettingsWidget::onAutoRunOnStartUpChanged);
    connect(ui.keepDialogOnErrorOccurredCb, &QCheckBox::toggled, this, &SettingsWidget::onKeepDialogOnErrorOccurredChanged);
    connect(ui.removeToTrashCb, &QCheckBox::toggled, this, &SettingsWidget::onRemoveToTrashChanged);
    connect(ui.patternLinkModeComboBox, &QComboBox::currentIndexChanged, this, &SettingsWidget::onPatternsChanged);
    connect(ui.symlinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsWidget::onSymlinkHotkeyChanged);
    connect(ui.hardlinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsWidget::onHardlinkHotkeyChanged);
    connect(ui.patternLinkHotkeyInputer, &KeyCombinationInputer::keyCombinationChanged, this, &SettingsWidget::onPatternLinkHotkeyChanged);

    updateText();
}

void SettingsWidget::updateText()
{
    setWindowTitle(EASYTR("SettingsWidget.WindowTitle"));

    ui.renamePatternGroupBox->setTitle(EASYTR("SettingsWidget.RenamePatternGroupBox.Title"));

    ui.languageText->setText(EASYTR("SettingsWidget.Text.Language"));
    ui.autoRunOnStartUpCb->setText(EASYTR("SettingsWidget.CheckBox.AutoRunOnStartUp"));
    ui.keepDialogOnErrorOccurredCb->setText(EASYTR("SettingsWidget.CheckBox.KeepDialogOnErrorOccurred"));
    ui.removeToTrashCb->setText(EASYTR("SettingsWidget.CheckBox.RemoveToTrash"));
    ui.patternLinkModeLabel->setText(EASYTR("SettingsWidget.Text.PatternsLinkLabel"));
    ui.renamePatternLe->setPlaceholderText(EASYTR("SettingsWidget.LineEdit.RenamePattern.Placeholder"));
    ui.renamePatternTips->setText(EASYTR("SettingsWidget.Text.RenamePatternTips"));
    ui.symlinkHotkeyText->setText(EASYTR("SettingsWidget.Text.SymlinkHotkey"));
    ui.hardlinkHotkeyText->setText(EASYTR("SettingsWidget.Text.HardlinkHotkey"));
    ui.patternLinkHotkeyText->setText(EASYTR("SettingsWidget.Text.PatternLinkHotkey"));
    ui.symlinkHotkeyInputer->setToolTip(EASYTR("SettingsWidget.KeyCombinationInputer.ToolTip"));
    ui.hardlinkHotkeyInputer->setToolTip(EASYTR("SettingsWidget.KeyCombinationInputer.ToolTip"));
    ui.patternLinkHotkeyInputer->setToolTip(EASYTR("SettingsWidget.KeyCombinationInputer.ToolTip"));

    ui.languageComboBox->setItemText(0, EASYTR("SettingsWidget.ComboBox.ItemText.English"));
    ui.languageComboBox->setItemText(1, EASYTR("SettingsWidget.ComboBox.ItemText.Chinese"));

    ui.patternLinkModeComboBox->setItemText(0, EASYTR("SettingsWidget.ComboBox.ItemText.Superficial"));
    ui.patternLinkModeComboBox->setItemText(1, EASYTR("SettingsWidget.ComboBox.ItemText.Hash"));
}

void SettingsWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QWidget::changeEvent(event);
}

bool SettingsWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == event->FocusOut)
        onRenamePatternChanged(ui.renamePatternLe->text());
    return QWidget::eventFilter(obj, event);
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

void SettingsWidget::onPatternsChanged(int index)
{
    switch (index)
    {
        case 0:
            settings_.patterns = SUPERFICIAL_PATTERNS;
            break;
        case 1:
            settings_.patterns = HASH_PATTERNS;
            break;
        default:
            return;
    }
    emit settingsChanged(settings_);
}

void SettingsWidget::onRenamePatternChanged(QString renamePattern)
{
    if (!isLegalRenamePattern(renamePattern))
    {
        QMessageBox msgBox(
            QMessageBox::Warning,
            EASYTR("Common.Warning"),
            EASYTR("SettingsWidget.MessageBox.IllegalRenamePattern.Text"),
            QMessageBox::Ok,
            this
        );
        msgBox.exec();
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
    // 由于使用的是Hook GHM，可以同时存在多个相同的全局热键，故需要要额外判断当前新热键是否已重复。
    if (qkc == ui.hardlinkHotkeyInputer->keyCombination() ||
        qkc == ui.patternLinkHotkeyInputer->keyCombination())
    {
        // Roll back
        ui.symlinkHotkeyInputer->setKeyCombination(gbhkKcToQtKc(settings_.symlinkHotkey));
        alertSameHotkey();
        return;
    }

    settings_.symlinkHotkey = qtKcToGbhkKc(qkc);
    emit settingsChanged(settings_);
}

void SettingsWidget::onHardlinkHotkeyChanged(QKeyCombination qkc)
{
    if (qkc == ui.symlinkHotkeyInputer->keyCombination() ||
        qkc == ui.patternLinkHotkeyInputer->keyCombination())
    {
        // Roll back
        ui.hardlinkHotkeyInputer->setKeyCombination(gbhkKcToQtKc(settings_.hardlinkHotkey));
        alertSameHotkey();
        return;
    }

    settings_.hardlinkHotkey = qtKcToGbhkKc(qkc);
    emit settingsChanged(settings_);
}

void SettingsWidget::onPatternLinkHotkeyChanged(QKeyCombination qkc)
{
    if (qkc == ui.symlinkHotkeyInputer->keyCombination() ||
        qkc == ui.hardlinkHotkeyInputer->keyCombination())
    {
        // Roll back
        ui.patternLinkHotkeyInputer->setKeyCombination(gbhkKcToQtKc(settings_.patternLinkHotkey));
        alertSameHotkey();
        return;
    }

    settings_.patternLinkHotkey = qtKcToGbhkKc(qkc);
    emit settingsChanged(settings_);
}

void SettingsWidget::alertSameHotkey()
{
    QMessageBox msgBox(
        QMessageBox::Warning,
        EASYTR("Common.Warning"),
        EASYTR("SettingsWidget.MessageBox.AlertSameHotkey.Text"),
        QMessageBox::Ok,
        this
    );
    msgBox.exec();
}
