:REM 用于清理程序注册表条目（Used for cleaning up program registry entries）

@echo off
reg delete "HKCU\Software\JaderoChan\EasyLinks" /f
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "EasyLins" /f
pause
