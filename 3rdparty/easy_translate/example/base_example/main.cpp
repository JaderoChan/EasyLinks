#include <qapplication.h>

#include <easy_translate.hpp>

#include "mainwidget.h"
#include "language.h"

#define APP_LANGUAGES_FILEPATH "./language/language.json"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    easytr::setLanguages(APP_LANGUAGES_FILEPATH);
    setLanguage(currentLanguage());

    MainWidget w;
    w.show();

    int ret = a.exec();

    easytr::updateTranslationsFiles();

    return ret;
}
