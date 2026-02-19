#pragma once

#include <qdialog.h>
#include <qevent.h>
#include <qtimer.h>

#include "ui_require_permission_dialog.h"

class RequirePermissionDialog : public QDialog
{
public:
    enum Result
    {
        GotPermission,
        Exit,
        ForceContinue
    };

    explicit RequirePermissionDialog(QWidget *parent = nullptr);

    void accept() override;
    void reject() override;

protected:
    virtual void updateText();
    void changeEvent(QEvent* event) override;

    void onTimeOut();
    void onGotoGrantPermissionBtnClicked();
    void onForceContinueBtnClicked();

private:
    Ui::RequirePermissionDialog ui;

    QTimer checkPermissionTimer_;
};
