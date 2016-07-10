@echo off

pushd %cd%
cd ..\..\
set SolutionDir=%cd%\
popd

set exePath=
if exist %SolutionDir%Win32\Debug\testCScript.exe (
 set exePath=%SolutionDir%Win32\Debug\testCScript.exe
 goto startProg
)
if exist %SolutionDir%Win32\Release\testCScript.exe (
 set exePath=%SolutionDir%Win32\Release\testCScript.exe
 goto startProg
)
if exist %SolutionDir%x64\Debug\testCScript.exe (
 set exePath=%SolutionDir%x64\Debug\testCScript.exe
 goto startProg
)
if exist %SolutionDir%x64\Release\testCScript.exe (
 set exePath=%SolutionDir%x64\Release\testCScript.exe
 goto startProg
)

:startProg
%exePath% /sf makeDevPackage.c %SolutionDir% win32 debug
%exePath% /sf makeDevPackage.c %SolutionDir% win32 release
%exePath% /sf makeDevPackage.c %SolutionDir% x64 debug
%exePath% /sf makeDevPackage.c %SolutionDir% x64 release

pause