#pragma once

#include <qdialog.h>

#include "ui_settings_dialog.h"
#include "language.h"
#include "settings.h"

class SettingsDialog : public QDialog
{
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void symlinkHotkeyChanged(gbhk::KeyCombination);
    void hardlinkHotkeyChanged(gbhk::KeyCombination);

protected:
    virtual void updateText();

    void onLanguageChanged(int index);
    void onAutoRunOnStartUpChanged(bool enable);
    void onKeepDialogOnErrorOccurredChanged(bool enable);
    void onRemoveToTrashChanged(bool enable);
    void onSymlinkHotkeyChanged(QKeyCombination qkc);
    void onHardlinkHotkeyChanged(QKeyCombination qkc);

private:
    void alertSameHotkey();

    Ui::SettingsDialog ui;
    Settings settings_;
};
