#pragma once

#include <qwidget.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qlabel.h>

class MainWidget : public QWidget
{
public:
    explicit MainWidget(QWidget* parent = nullptr);

protected:
    virtual void updateText();

    void changeEvent(QEvent* event) override;

protected slots:
    void onToggleLanguageBtnPressed();

private:
    QLabel textLbl_;
    QPushButton toggleLanguageBtn_;
};
