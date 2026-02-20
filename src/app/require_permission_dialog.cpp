#include "require_permission_dialog.h"

#include <easy_translate.hpp>

#include "platforms/permission_manager.h"

RequirePermissionDialog::RequirePermissionDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    checkPermissionTimer_.start(20);

    connect(&checkPermissionTimer_, &QTimer::timeout, this, &RequirePermissionDialog::onTimeOut);
    connect(ui.gotoGrantPermissionBtn, &QPushButton::clicked,
        this, &RequirePermissionDialog::onGotoGrantPermissionBtnClicked);
    connect(ui.exitBtn, &QPushButton::clicked, this, &RequirePermissionDialog::reject);
    connect(ui.forceContinueBtn, &QPushButton::clicked,
        this, &RequirePermissionDialog::onForceContinueBtnClicked);

    updateText();
}

void RequirePermissionDialog::accept()
{
    done(RequirePermissionDialog::GotPermission);
}

void RequirePermissionDialog::reject()
{
    done(RequirePermissionDialog::Exit);
}

void RequirePermissionDialog::updateText()
{
    setWindowTitle(EASYTR("Common.Warning"));
#ifdef Q_OS_MAC
    ui.messageTextLbl->setText(EASYTR("RequirePermissionDialog.Text.Message.Mac"));
#else
    ui.messageTextLbl->setText(EASYTR("RequirePermissionDialog.Text.Message"));
#endif // Q_OS_MAC
    ui.gotoGrantPermissionBtn->setText(EASYTR("RequirePermissionDialog.Button.GotoGrantPermission"));
    ui.exitBtn->setText(EASYTR("RequirePermissionDialog.Button.Exit"));
    ui.forceContinueBtn->setText(EASYTR("RequirePermissionDialog.Button.ForceContinue"));
    ui.forceContinueBtn->setToolTip(EASYTR("RequirePermissionDialog.Button.ForceContinue.ToolTip"));
}

void RequirePermissionDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QDialog::changeEvent(event);
}

void RequirePermissionDialog::onTimeOut()
{
    if (PermissionManager::hasPermission())
        accept();
}

void RequirePermissionDialog::onGotoGrantPermissionBtnClicked()
{
    PermissionManager::requestPermission();
}

void RequirePermissionDialog::onForceContinueBtnClicked()
{
    done(RequirePermissionDialog::ForceContinue);
}
