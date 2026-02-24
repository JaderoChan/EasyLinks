#include "permission_manager.h"

#include <cstdlib>  // system

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>

bool PermissionManager::hasPermission()
{
    // TODO: Check Finder and System events automation permission.
    return AXIsProcessTrustedWithOptions(NULL);
}

void PermissionManager::requestPermission()
{
    // MacOS下需要“辅助功能”权限以支持Hook全局热键。
    CFStringRef keys[] = {kAXTrustedCheckOptionPrompt};
    CFTypeRef values[] = {kCFBooleanTrue};
    CFDictionaryRef options = CFDictionaryCreate(
        kCFAllocatorDefault,
        (const void**) keys, (const void**) values, 1,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );

    AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
}
