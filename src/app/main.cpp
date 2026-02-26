#include <qapplication.h>
#include <qdir.h>
#include <qlockfile.h>

#include <easy_translate.hpp>

#include "config.h"
#include "app_manager.h"
#include "file_logger.h"
#include "language.h"
#include "require_permission_dialog.h"
#include "settings.h"
#include "logo_icon.h"
#include "platforms/permission_manager.h"
#include "utils/logging.h"

int main(int argc, char* argv[])
{
    QLockFile lock(QDir::temp().absoluteFilePath(APP_LOCK_FILEPATH));
    if (lock.isLocked() || !lock.tryLock(200))
    {
        debugOut(qCritical(), "[Start] Another instance is already running.");
        return 1;
    }

    // 设置程序全局属性
    QApplication a(argc, argv);
    a.setOrganizationDomain(APP_ORGANIZATION_DOMAIN);
    a.setOrganizationName(APP_ORGANIZATION);
    a.setApplicationName(APP_TITLE);
    a.setApplicationVersion(APP_VERSION);
    a.setWindowIcon(getLogoIcon());
    a.setQuitOnLastWindowClosed(false);

    FileLogger& fileLogger = FileLogger::getInstance();
    if (!fileLogger.setup(APP_LOG_FILEPATH))
        debugOut(qWarning(), "[Start] Failed to setup file logger.");

    // 设置语言
    {
        Settings settings = loadSettings();
        setLanguage(settings.language);
    }

    // 检查应用权限
    if (!PermissionManager::hasPermission())
    {
        debugOut(qInfo(), "[Start] No permission, try request.");

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

    // 应用主体
    AppManager mgr;

    int ret = a.exec();

    // 更新翻译文件（实际上由编译选项`UPDATE_TRANSLATIONS_FILES`决定是否真正更新）
    easytr::updateTranslationsFiles();

    return ret;
}
