@ECHO OFF

set binpath=%~dp0\..\bin

ECHO.
ECHO Removing bin directory...
rmdir %binpath% /s /q

ECHO.
ECHO Creating bin directory...
mkdir %binpath%

ECHO.
ECHO Copying Qt...
copy %QTPATH%\bin\Qt5Core*.dll %binpath%\
copy %QTPATH%\bin\Qt5Gui*.dll  %binpath%\
copy %QTPATH%\bin\Qt5Network*.dll  %binpath%\
copy %QTPATH%\bin\Qt5OpenGL*.dll  %binpath%\
copy %QTPATH%\bin\Qt5Widgets*.dll  %binpath%\
copy %QTPATH%\bin\Qt5PrintSupport*.dll  %binpath%\
xcopy %QTPATH%\plugins\imageformats\qjpeg*.dll %binpath%\imageformats\
xcopy %QTPATH%\plugins\platforms\qwindows*.dll %binpath%\platforms\
xcopy %QTPATH%\plugins\styles\qwindowsvistastyle*.dll %binpath%\styles\

REM ECHO.
REM ECHO Copying OpenSSL...
REM copy %QTPATH%\..\..\Tools\OpenSSL\Win_x64\bin\*.dll %binpath%\
REM copy C:\Windows\System32\msvcr100.dll %~dp0bin\

ECHO.
ECHO Copying VC Redistributables...
copy "%MSVC2019_REDIST_PATH%\concrt140.dll" %binpath%\
copy "%MSVC2019_REDIST_PATH%\msvcp140.dll" %binpath%\
copy "%MSVC2019_REDIST_PATH%\msvcp140_1.dll" %binpath%\
copy "%MSVC2019_REDIST_PATH%\vcruntime140.dll" %binpath%\
copy "%MSVC2019_REDIST_PATH%\vcruntime140_1.dll" %binpath%\

ECHO.
pause