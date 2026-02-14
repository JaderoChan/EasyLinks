#include <qapplication.h>
#include <qdir.h>
#include <qlockfile.h>

#include <easy_translate.hpp>

#include "config.h"
#include "app_manager.h"

int main(int argc, char* argv[])
{
    QLockFile lock(QDir::temp().absoluteFilePath(APP_LOCK_FILEPATH));
    if (lock.isLocked() || !lock.tryLock(200))
        return 1;

    QApplication a(argc, argv);
    a.setOrganizationDomain(APP_ORGANIZATION_DOMAIN);
    a.setOrganizationName(APP_ORGANIZATION);
    a.setApplicationName(APP_TITLE);
    a.setApplicationVersion(APP_VERSION);
    a.setWindowIcon(QIcon(":/icons/app.ico"));
    a.setQuitOnLastWindowClosed(false);

    AppManager mgr;

    int ret = a.exec();

    easytr::updateTranslationsFiles();

    return ret;
}
