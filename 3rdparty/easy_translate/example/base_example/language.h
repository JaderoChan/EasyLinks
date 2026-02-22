#pragma once

#include <qstring.h>

enum LanguageID
{
    LANG_EN,
    LANG_ZH
};

QString getLanguageIdStr(LanguageID langId);

void setLanguage(LanguageID langId);

LanguageID currentLanguage();
