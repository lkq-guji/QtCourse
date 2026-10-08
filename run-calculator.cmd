@echo off
setlocal
set "PATH=E:\QT\6.11.1\mingw_64\bin;E:\QT\Tools\mingw1310_64\bin;%PATH%"
if not exist "%~dp0calculator\build\debug\calculator.exe" (
    echo Please build calculator/calculator.pro in Qt Creator first.
    pause
    exit /b 1
)
start "" "%~dp0calculator\build\debug\calculator.exe"
