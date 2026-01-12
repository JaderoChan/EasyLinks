#include <qapplication.h>
#include <qdir.h>
#include <qlockfile.h>

#include <easy_translate.hpp>

#include "config.h"
#include "language.h"
#include "filelink/filelink_manager.h"

int main(int argc, char* argv[])
{
    QLockFile lock(QDir::temp().absoluteFilePath(APP_LOCK_FILENAME));
    if (lock.isLocked() || !lock.tryLock(500))
        return 0;

    QApplication a(argc, argv);
    // a.setQuitOnLastWindowClosed(false);

    if (!setLanguage(APPLANG_ZH))
        qDebug() << "Failed set the language";

    // test
    QString source("E:/05_Data");
    QString target("E:/06_TMP/Test");
    FileLinkManager m;
    m.createLinks(LT_HARDLINK, {source}, target);

    int ret = a.exec();

    easytr::updateTranslationsFiles();

    return ret;
}
