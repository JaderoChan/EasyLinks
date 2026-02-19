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
    // 确保单例运行
    QLockFile lock(QDir::temp().absoluteFilePath(APP_LOCK_FILEPATH));
    if (lock.isLocked() || !lock.tryLock(200))
        return 1;

    // 设置程序全局属性
    QApplication a(argc, argv);
    a.setOrganizationDomain(APP_ORGANIZATION_DOMAIN);
    a.setOrganizationName(APP_ORGANIZATION);
    a.setApplicationName(APP_TITLE);
    a.setApplicationVersion(APP_VERSION);
    a.setWindowIcon(QIcon(":/icons/app.ico"));
    a.setQuitOnLastWindowClosed(false);

    // 配置日志输出文件
    QDir logDir = QDir(APP_LOG_DIRPATH);
    if (!logDir.exists())
    {
        if (!logDir.mkpath("."))
            qCritical() << "Failed to create log directory:" << APP_LOG_DIRPATH;
    }

    FileLogManager& logMgr = FileLogManager::getInstance();
    if (!logMgr.setup(APP_LOG_FILEPATH))
        qCritical() << "Failed to set up log file:" << APP_LOG_FILEPATH;

    // 应用主体
    AppManager mgr;

    // 检查应用权限
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

    int ret = a.exec();

    // 更新翻译文件（实际上由编译选项`UPDATE_TRANSLATIONS_FILES`决定是否真正更新）
    easytr::updateTranslationsFiles();

    return ret;
}
