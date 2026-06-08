osascript -e 'tell application "System Events"
    set frontAppBundleId to bundle identifier of first application process where it is frontmost
end tell

if frontAppBundleId is not "com.apple.finder" then
    error "Front application is not Finder"
end if

tell application "Finder"
    if (count of Finder windows) = 0 then
        error "No Finder window is open"
    end if

    set targetFolder to (quoted form of POSIX path of (target of front window as alias))
    if targetFolder is missing value then
        error "Finder window has no valid target"
    end if

    return targetFolder
end tell'
