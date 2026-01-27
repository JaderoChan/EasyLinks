#include "language.h"

#include <qapplication.h>
#include <qevent.h>

#include <easy_translate.hpp>

#include "config.h"

QString languageStringId(Language lang)
{
    switch (lang)
    {
        case LANG_EN:   return "EN";
        case LANG_ZH:   return "ZH";
        default:        return "";
    }
}

Language currentSystemLang()
{
    switch (QLocale::system().language())
    {
        case QLocale::Language::English:    return LANG_EN;
        case QLocale::Language::Chinese:    return LANG_ZH;
        default:                            return LANG_EN;
    }
}

bool setLanguage(Language lang)
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
