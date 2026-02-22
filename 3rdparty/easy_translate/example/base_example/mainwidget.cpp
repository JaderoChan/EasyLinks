#include "mainwidget.h"

#include <qlayout.h>

#include <easy_translate.hpp>

#include "language.h"

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(300, 240);
    textLbl_.setAlignment(Qt::AlignCenter);

    // Init the UI.
    setLayout(new QVBoxLayout());
    layout()->addWidget(&textLbl_);
    layout()->addWidget(&toggleLanguageBtn_);

    connect(&toggleLanguageBtn_, &QPushButton::clicked, this, &MainWidget::onToggleLanguageBtnPressed);

    updateText();
}

void MainWidget::updateText()
{
    setWindowTitle(EASYTR("EasyTranslateExample"));
    textLbl_.setText(EASYTR("DisplayText"));
    toggleLanguageBtn_.setText(EASYTR("ToggleLanguage"));
}

void MainWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QWidget::changeEvent(event);
}

void MainWidget::onToggleLanguageBtnPressed()
{
    LanguageID langId = LANG_EN;
    switch (currentLanguage())
    {
        case LANG_EN:
            langId = LANG_ZH;
            break;
        case LANG_ZH:
            langId = LANG_EN;
            break;
        default:
            break;
    }
    setLanguage(langId);
}
