:REM 用于清理程序注册表/本地缓存条目（Used for cleaning up program registry entries）

@echo off
reg delete "HKCU\Software\JaderoChan\EasyLinks" /f 2>&1 >nul
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "EasyLins" /f 2>&1 >nul
set "PROGRAM_DATA_DIR=%APPDATA%"/JaderoChan/EasyLinks
rd /s /q "%PROGRAM_DATA_DIR%" 2>&1 >nul
pause
