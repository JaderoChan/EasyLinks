#pragma once

#include <qevent.h>
#include <qaction.h>
#include <qmenu.h>
#include <qsystemtrayicon.h>

class SystemTrayIcon : public QSystemTrayIcon
{
public:
    explicit SystemTrayIcon(QObject* parent = nullptr);

signals:
    void settingsActionTriggered();
    void aboutActionTriggered();
    void exitActionTriggered();

protected:
    virtual void updateText();
    bool eventFilter(QObject* obj, QEvent* event) override;

    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QMenu menu_;
    QAction settingsAction_;
    QAction aboutAction_;
    QAction exitAction_;
};
