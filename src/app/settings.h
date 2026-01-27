#pragma once

#include <qstring.h>
#include <qobject.h>

#include <global_hotkey/key_combination.hpp>

#include "language.h"
#include "filelink/link_config.h"

struct Settings
{
    bool autoRunOnStartUp = false;
    Language language;
    LinkConfig linkConfig;
    gbhk::KeyCombination symlinkHotkey  = "Ctrl+S";
    gbhk::KeyCombination hardlinkHotkey = "Ctrl+H";
};

Settings loadSettings();

void saveSettings(const Settings& settings);
