#pragma once

#include <qstring.h>

#include <global_hotkey/key_combination.hpp>

#include "language.h"
#include "filelink/link_config.h"
#include "filelink/pattern.h"

constexpr Patterns SUPERFICIAL_PATTERNS = PATTERN_SAME_NAME | PATTERN_SAME_SIZE | PATTERN_SAME_PERM;
constexpr Patterns HASH_PATTERNS        = PATTERN_SAME_HASH;
constexpr Patterns DEFAULT_PATTERNS     = SUPERFICIAL_PATTERNS;

struct Settings
{
    bool       autoRunOnStartUp;
    Language   language;
    LinkConfig linkConfig;
    gbhk::KeyCombination symlinkHotkey;
    gbhk::KeyCombination hardlinkHotkey;
    gbhk::KeyCombination patternLinkHotkey;

    Patterns   patterns;
    bool       needReview;
};

Settings loadSettings();

void saveSettings(const Settings& settings);
