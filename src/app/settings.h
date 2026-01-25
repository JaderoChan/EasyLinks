#pragma once

#include <qsettings.h>
#include <qstring.h>

#include <global_hotkey/key_combination.hpp>

#include "filelink/link_config.h"

struct Settings
{
    bool autoRunOnStartUp = false;
    gbhk::KeyCombination symlinkHotkey  = "Ctrl+S";
    gbhk::KeyCombination hardlinkHotkey = "Ctrl+H";
    LinkConfig linkConfig;
};
