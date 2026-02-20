#pragma once

#include <qevent.h>
#include <qwidget.h>

#include "ui_settings_widget.h"
#include "settings.h"

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(const Settings& settings, QWidget* parent = nullptr);

signals:
    void settingsChanged(Settings settings);

protected:
    virtual void updateText();
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void onLanguageChanged(int index);
    void onAutoRunOnStartUpChanged(bool enable);
    void onKeepDialogOnErrorOccurredChanged(bool enable);
    void onRemoveToTrashChanged(bool enable);
    void onRenamePatternChanged(QString renamePattern);
    void onSymlinkHotkeyChanged(QKeyCombination qkc);
    void onHardlinkHotkeyChanged(QKeyCombination qkc);

private:
    void alertSameHotkey();

    Ui::SettingsWidget ui;
    Settings settings_;
};
