#pragma once

#include <qdialog.h>
#include <qevent.h>

#include "ui_about_dialog.h"

class AboutDialog : public QDialog
{
public:
    explicit AboutDialog(QWidget* parent = nullptr);

protected:
    virtual void updateText();
    void changeEvent(QEvent* event) override;

private:
    Ui::AboutDialog ui;
};
