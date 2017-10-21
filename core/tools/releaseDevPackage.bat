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
%exePath% /sf makeDevPackage.c %SolutionDir% win32 StaticRelease
%exePath% /sf makeDevPackage.c %SolutionDir% x64 debug
%exePath% /sf makeDevPackage.c %SolutionDir% x64 release

echo copy all other head file for notstd¡¢libxml2¡¢libiconv¡¢libzlib

set TargetRoot=%SolutionDir%devRelease\

rem copy zlib
mkdir %TargetRoot%include\zlib
mkdir %TargetRoot%include\zlib\contrib
mkdir %TargetRoot%include\zlib\contrib\minizip
copy %SolutionDir%thirdparty\libzlib\zlib-1.2.8\*.h %TargetRoot%include\zlib
copy %SolutionDir%thirdparty\libzlib\zlib-1.2.8\contrib\minizip\*.h %TargetRoot%include\zlib\contrib\minizip

rem copy libiconv
set iconvRoot=%TargetRoot%include\libiconv\
mkdir %iconvRoot%
mkdir %iconvRoot%iconv
copy %SolutionDir%thirdparty\libiconv\iconv\*.h %iconvRoot%iconv

rem copy libxml2
xcopy /E /I /Y %SolutionDir%thirdparty\libxml2-2.9.2\include\libxml\*.* %TargetRoot%include\libxml

rem copy libnotstd
rem xcopy /E /I /Y %SolutionDir%core\notstd\
for /f %%i in ('dir /b %SolutionDir%core\notstd\*.h') do (
 copy /Y %SolutionDir%core\notstd\%%i %TargetRoot%include\notstd\
)
for /f %%i in ('dir /b %SolutionDir%core\notstd\*.hpp') do (
 copy /Y %SolutionDir%core\notstd\%%i %TargetRoot%include\notstd\
)

pause
