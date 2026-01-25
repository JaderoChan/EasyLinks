#include <qapplication.h>
#include <qlockfile.h>

#include <easy_translate.hpp>

#include "config.h"
#include "language.h"
#include "filelink/manager.h"

int main(int argc, char* argv[])
{
    QLockFile lock(APP_LOCK_FILENAME);
    if (lock.isLocked() || !lock.tryLock(500))
        return 0;

    QApplication a(argc, argv);
    // a.setQuitOnLastWindowClosed(false);

    if (!setLanguage(APPLANG_ZH))
        qDebug() << "Failed set the language";

    FileLinkManager m(LT_HARDLINK, {"E:/05_Data"}, "E:/06_TMP/Test");
    m.start();

    int ret = a.exec();

    easytr::updateTranslationsFiles();

    return ret;
}
