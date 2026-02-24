:REM 用于清理程序注册表/本地缓存条目（Used for cleaning up program registry entries）

@echo off
reg delete "HKCU\Software\JaderoChan\EasyLinks" /f
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "EasyLins" /f
set "PROGRAM_DATA_DIR=%APPDATA%/JaderoChan/EasyLinks"
rd /s /q "%PROGRAM_DATA_DIR%"
pause
