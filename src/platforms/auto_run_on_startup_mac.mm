#include "auto_run_on_startup.h"

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

#include <qdebug.h>

// AI Generate

bool isAutoRunOnStartUp()
{
    @autoreleasepool
    {
        SMAppService *service = SMAppService.mainAppService;
        return service.status == SMAppServiceStatusEnabled;
    }
}

bool setAutoRunOnStartUp(bool enable)
{
    @autoreleasepool
    {
        NSError *error = nil;
        BOOL success;

        if (enable)
            success = [SMAppService.mainAppService registerAndReturnError:&error];
        else
            success = [SMAppService.mainAppService unregisterAndReturnError:&error];

        if (!success && error)
            qDebug() << "Failed to " << (enable ? "register" : "unregister")
                     << " login item:" << [error.localizedDescription UTF8String];

        return success;
    }
}
