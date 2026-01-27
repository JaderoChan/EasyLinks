#include <qapplication.h>
#include <qdir.h>
#include <qlockfile.h>

#include <easy_translate.hpp>

#include "config.h"
#include "language.h"
#include "settings.h"
#include "system_tray_icon.h"
#include "auto_run_on_startup.h"

int main(int argc, char* argv[])
{
    QLockFile lock(QDir::temp().absoluteFilePath(APP_LOCK_FILENAME));
    if (lock.isLocked() || !lock.tryLock(250))
        return 1;

    QApplication a(argc, argv);
    a.setOrganizationName(APP_ORGANIZATION);
    a.setApplicationName(APP_TITLE);
    a.setQuitOnLastWindowClosed(false);

    auto settings = loadSettings();
    if (!setLanguage(settings.language));
        qDebug() << QString("Failed to set language to %1").arg(languageStringId(settings.language));

    if (settings.autoRunOnStartUp != isAutoRunOnStartUp())
    {
        if (!setAutoRunOnStartUp(settings.autoRunOnStartUp))
            qDebug() << QString("Failed to %1 auto run on start up").arg(settings.autoRunOnStartUp ? "set" : "unset");
    }

    SystemTrayIcon sti;
    sti.show();
    a.installEventFilter(&sti);

    int ret = a.exec();

    easytr::updateTranslationsFiles();

    return ret;
}
