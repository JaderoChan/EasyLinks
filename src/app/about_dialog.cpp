#include "about_dialog.h"

#include <easy_translate.hpp>

#include "config.h"

AboutDialog::AboutDialog(QWidget* parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    ui.icon->setPixmap(QPixmap(":/icons/app.ico"));
    ui.versionLbl->setText(APP_VERSION);
    ui.copyrightLbl->setText(APP_COPYRIGHT_TEXT);
    ui.authorValueLbl->setText(APP_AUTHOR);
    ui.contactValueLbl->setText(APP_AUTHOR_EMAIL);

    updateText();
}

void AboutDialog::showAndActivate()
{
    show();
    raise();
    activateWindow();
}

void AboutDialog::updateText()
{
    setWindowTitle(EASYTR("AboutDialog.WindowTitle"));
    ui.titleLbl->setText(EASYTR("App.Title"));
    ui.authorTextLbl->setText(EASYTR("AboutDialog.AuthorText"));
    ui.contactTextLbl->setText(EASYTR("AboutDialog.ContactText"));
}

void AboutDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QDialog::changeEvent(event);
}
