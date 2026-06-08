#pragma once

#include <qobject.h>

#include <global_hotkey/global_hotkey.hpp>

#include "filelink/types.h"
#include "settings.h"

class HotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();

    void setSettings(const Settings& settings);

signals:
    void shouldLinks(LinkType linkType);
    void shouldPatternLink();
    void linkCompleted(LinkType linkType, QString targetDir, LinkStats stats);
    void patternLinkTriggered(QString dir);

private:
    void links(LinkType linkType);
    void patternLink();

    Settings settings_;
    gbhk::GlobalHotkeyManager& ghm_;
};
