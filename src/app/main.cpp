#include <qapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qlockfile.h>

#include <easy_translate.hpp>

#include "config.h"
#include "app_manager.h"
#include "file_log_manager.h"
#include "require_permission_dialog.h"
#include "platforms/permission_manager.h"

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

    QDir logDir = QDir(APP_LOG_DIRPATH);
    if (!logDir.exists())
    {
        if (!logDir.mkpath("."))
            qCritical() << "Failed to create log directory:" << APP_LOG_DIRPATH;
    }

    FileLogManager& logMgr = FileLogManager::getInstance();
    if (!logMgr.setup(APP_LOG_FILEPATH))
        qCritical() << "Failed to set up log file:" << APP_LOG_FILEPATH;

    if (!PermissionManager::hasPermission())
    {
        qInfo() << "No permission, requesting...";

        RequirePermissionDialog dlg;
        int ret = dlg.exec();
        switch (ret)
        {
            case RequirePermissionDialog::GotPermission:
            case RequirePermissionDialog::ForceContinue:
                break;
            case RequirePermissionDialog::Exit:
                return 0;
            default:
                break;
        }
    }

    AppManager mgr;

    int ret = a.exec();

    easytr::updateTranslationsFiles();

    return ret;
}
