#pragma once

#include <qstring.h>

enum Language : char
{
    LANG_EN = 0,
    LANG_ZH
};

QString languageStringId(Language lang);

// 如果应用程序不支持当前系统语言则返回`LANG_EN`。
Language currentSystemLang();

bool setLanguage(Language lang);
