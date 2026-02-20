#include "auto_run_on_startup.h"

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

#include <qcoreapplication.h>
#include <qdir.h>

// AI Generate

bool isAutoRunOnStartUp()
{
    @autoreleasepool
    {
        NSString *appPath = [[NSBundle mainBundle] bundlePath];

        if (@available(macOS 13.0, *))
        {
            // macOS 13+ 使用新的 API
            SMAppService *service = SMAppService.mainAppService;
            return service.status == SMAppServiceStatusEnabled;
        }
        else
        {
            // macOS 10.10 - 12.x 使用旧的 API
            LSSharedFileListRef loginItems =
                LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
            if (!loginItems)
                return false;

            BOOL found = NO;
            UInt32 seedValue;
            CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);

            for (id item in (__bridge NSArray *) loginItemsArray)
            {
                LSSharedFileListItemRef itemRef = (__bridge LSSharedFileListItemRef) item;
                CFURLRef url = NULL;

                if (LSSharedFileListItemResolve(itemRef, 0, &url, NULL) == noErr)
                {
                    NSString *itemPath = [(__bridge NSURL *)url path];
                    if ([itemPath isEqualToString:appPath])
                    {
                        found = YES;
                        if (url) CFRelease(url);
                        break;
                    }
                    if (url) CFRelease(url);
                }
            }

            if (loginItemsArray) CFRelease(loginItemsArray);
            CFRelease(loginItems);
            return found;
        }
    }
}

bool setAutoRunOnStartUp(bool enable)
{
    @autoreleasepool
    {
        NSString *appPath = [[NSBundle mainBundle] bundlePath];
        NSURL *bundleURL = [NSURL fileURLWithPath:appPath];

        if (@available(macOS 13.0, *))
        {
            // macOS 13+ 使用新的 API
            NSError *error = nil;

            if (enable)
            {
                BOOL success = [SMAppService.mainAppService registerAndReturnError:&error];
                if (!success && error)
                    NSLog(@"Failed to register login item: %@", error.localizedDescription);
                return success;
            }
            else
            {
                BOOL success = [SMAppService.mainAppService unregisterAndReturnError:&error];
                if (!success && error)
                    NSLog(@"Failed to unregister login item: %@", error.localizedDescription);
                return success;
            }
        }
        else
        {
            // macOS 10.10 - 12.x 使用旧的 API
            LSSharedFileListRef loginItems =
            LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
            if (!loginItems)
                return false;

            if (enable)
            {
                // 添加到登录项
                LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(
                    loginItems,
                    kLSSharedFileListItemLast,
                    NULL,
                    NULL,
                    (__bridge CFURLRef)bundleURL,
                    NULL,
                    NULL);
                if (item)
                    CFRelease(item);
                CFRelease(loginItems);
                return item != NULL;
            }
            else
            {
                // 从登录项移除
                UInt32 seedValue;
                CFArrayRef loginItemsArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);

                for (id item in (__bridge NSArray *)loginItemsArray)
                {
                    LSSharedFileListItemRef itemRef = (__bridge LSSharedFileListItemRef)item;
                    CFURLRef url = NULL;

                    if (LSSharedFileListItemResolve(itemRef, 0, &url, NULL) == noErr)
                    {
                        NSString *itemPath = [(__bridge NSURL *)url path];
                        if ([itemPath isEqualToString:appPath])
                        {
                            LSSharedFileListItemRemove(loginItems, itemRef);
                            if (url) CFRelease(url);
                            break;
                        }
                        if (url) CFRelease(url);
                    }
                }

                if (loginItemsArray) CFRelease(loginItemsArray);
                CFRelease(loginItems);
                return true;
            }
        }
    }
}
