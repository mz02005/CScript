@echo off
set SVNINFO_PATHNAME=svnInfo.h
svn info > vtemp.txt
if %errorlevel% NEQ 0 (echo "get svn info fail"&& exit 1)
for /f "delims=: tokens=1,2" %%i in (vtemp.txt) do (
 if "%%i" EQU "Revision" (
  echo Found revision is %%i
  echo #pragma once > %SVNINFO_PATHNAME%
  echo #define SVN_VERSION %%j >> %SVNINFO_PATHNAME%
  exit 0
 )
)

echo Could not find revision information
echo #pragma once > %SVNINFO_PATHNAME%
echo #define SVN_VERSION -1 >> %SVNINFO_PATHNAME%
exit 0