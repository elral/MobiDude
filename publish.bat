@echo off
REM ----------------------------------------
REM folders and settings
set BUILD_DIR=bin\publish\Mobiflight
set ZIP_DIR=bin\publish
set RUNTIME=win-x64
set CONFIG=Release
set FRAMEWORK=net8.0-windows
REM ----------------------------------------

REM read version from csproj (only the main <Version> tag)
for /f "tokens=2 delims=<>" %%i in ('findstr /R /C:"^[ ]*<Version>[0-9]" MobiDude.csproj') do set VERSION=%%i

REM fallback if no version found
if "%VERSION%"=="" set VERSION=0.0.0

REM delete Build folder if existent
if exist %BUILD_DIR% rd /s /q %BUILD_DIR%

REM publish Project
dotnet publish MobiDude.csproj -c Release -r win-x64 --self-contained true --output %BUILD_DIR% ^
    /p:PublishSingleFile=true /p:IncludeAllContentForSelfExtract=true

REM Optional: delete .pdb files (not required for Release)
del /q %BUILD_DIR%\*.pdb >nul 2>&1

REM copy Data folder recursiv
xcopy /E /Y /I /F ".\Data" "%BUILD_DIR%\Data\"

REM copy Tools folder recursiv
xcopy /E /Y /I /F ".\Tools" "%BUILD_DIR%\Tools\"

REM create ZIP-file
set ZIP_NAME=MobiDude_%VERSION%.zip
powershell -Command "Compress-Archive -Path '%ZIP_DIR%\*' -DestinationPath '%ZIP_DIR%\%ZIP_NAME%' -Force"

echo ----------------------------------------
echo ? Published in: %BUILD_DIR%
echo ----------------------------------------

pause
