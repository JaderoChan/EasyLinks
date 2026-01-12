#include "language.h"

#include <qapplication.h>
#include <qevent.h>

#include <easy_translate.hpp>

#include "config.h"

static QString languageStringId(AppLanguage lang)
{
    switch (lang)
    {
        case APPLANG_EN:    return "EN";
        case APPLANG_ZH:    return "ZH";
        default:            return "";
    }
}

bool setLanguage(AppLanguage lang)
{
    easytr::setLanguages(APP_LANG_LIST_FILENAME);
    if (easytr::languages().empty())
    {
        qWarning() << "Invalid or empty Languages file";
        return false;
    }

    std::string id = languageStringId(lang).toStdString();
    if (!easytr::hasLanguage(id))
    {
        qWarning() << "Expected language is missing";
        return false;
    }

    easytr::setCurrentLanguage(id);

    QEvent event(QEvent::Type::LanguageChange);
    qApp->sendEvent(qApp, &event);

    return true;
}
