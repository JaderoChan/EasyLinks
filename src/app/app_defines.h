#pragma once

#include <qapplication.h>
#include <qstring.h>

#include "config.h"

#define APP_LANG_FILEPATH       "languages/languages.json"
#define APP_LOCK_FILEPATH       (APP_TITLE "-" APP_UUID ".lock")

static const QString APP_DIRPATH = QApplication::applicationDirPath();
#if defined(Q_OS_MAC) && defined(IS_MACOSX_BUNDLE)
    static const QString APP_RESOURCE_DIRPATH = APP_DIRPATH + "/../Resources";
#else
    static const QString APP_RESOURCE_DIRPATH = APP_DIRPATH;
#endif // Q_OS_MAC
