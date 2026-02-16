#include "permission_manager.h"

#include <ApplicationServices/ApplicationServices.h>

bool PermissionManager::hasPermission()
{
    return AXIsProcessTrusted();
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
