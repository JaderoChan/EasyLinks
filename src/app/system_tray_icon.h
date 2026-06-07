#pragma once

#include <qaction.h>
#include <qmenu.h>
#include <qsystemtrayicon.h>

class SystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTrayIcon(QObject* parent = nullptr);

    void updateText();

signals:
    void patternLinkActionTriggered();
    void settingsActionTriggered();
    void aboutActionTriggered();
    void openLogDirActionTriggered();
    void exitActionTriggered();

protected:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QMenu menu_;
    QAction patternLinkAction_;
    QAction settingsAction_;
    QAction aboutAction_;
    QAction openLogDirAction_;
    QAction exitAction_;
};
