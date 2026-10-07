@echo off
setlocal EnableExtensions DisableDelayedExpansion
title Beach Day - Build and Run
pushd "%~dp0"
if errorlevel 1 goto directory_error

set "PROJECT_EXE=%CD%\build\x64\Release\Project1.exe"
set "PROJECT_MSBUILD="
set "PROJECT_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

rem Restart our running scene before rebuilding, so its EXE is not locked.
rem Match the full path so another project's Project1.exe is never stopped.
powershell.exe -NoLogo -NoProfile -Command "$ErrorActionPreference='Stop'; try { $running=Get-Process -Name Project1 -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $env:PROJECT_EXE }; foreach ($p in $running) { Write-Host ('Closing the previous Beach Day instance: '+$p.Id); if ($p.MainWindowHandle -ne 0) { [void]$p.CloseMainWindow(); if ($p.WaitForExit(10000)) { continue } }; if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction Stop; if (-not $p.WaitForExit(5000)) { throw 'The previous instance did not exit.' } } }; exit 0 } catch { Write-Host ('Could not restart Beach Day: '+$_.Exception.Message); exit 1 }"
if errorlevel 1 goto process_error

if exist "%PROJECT_VSWHERE%" (
    for /f "usebackq delims=" %%M in (`"%PROJECT_VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "PROJECT_MSBUILD=%%M"
)
if not defined PROJECT_MSBUILD (
    for /f "delims=" %%M in ('where MSBuild.exe 2^>nul') do set "PROJECT_MSBUILD=%%M"
)
if not defined PROJECT_MSBUILD goto no_build_tools

echo Building Beach Day - Release x64...
rem Cleanup may remove build intermediates; MSBuild recreates them here.
if not exist "build" mkdir "build"
"%PROJECT_MSBUILD%" "Project1.vcxproj" /t:Build /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo /fl /flp:"logfile=build\launcher.log"
if errorlevel 1 goto build_error
if /i "%~1"=="--build-only" goto build_complete
goto launch

:no_build_tools
if /i "%~1"=="--build-only" goto tools_error
if not exist "%PROJECT_EXE%" goto tools_error
echo Visual Studio build tools not found. Running the existing Release build.
goto launch

:launch
if not exist "%PROJECT_EXE%" goto missing_exe
echo Opening Beach Day...
echo Lighthouse interior: F3 cutaway / exterior, F4 next floor.
echo Tower: T then Enter controls keeper; W/S climb / descend stairs.
echo V places keeper on gallery; S descends; E switches POV / cutaway.
echo Space starts auto climb / pauses; PageUp restarts; PageDown resets.
echo FPV: left-drag looks around; A/D walks the top gallery; Enter exits.
echo At the top, click LIGHTHOUSE LIGHT CONTROL: power, manual aim, tilt, sweep.
echo Click inside the 3D scene window before using keyboard controls.
echo Walking robot: Enter to control, Q charge / disconnect, Home locate station.
echo Updated report: 17 pages, including tower FPV and manual light controls.
echo Report file: build\report\Beach_Day_Project_Report.pdf
echo Updated slides: 10 slides; checked primitive meshes, buoyancy, UVs and shadows.
echo Slide file: build\Slides\2107077_Beach_Day_PowerPoint_Checked.pptx
echo Bangla slide guide: output\pdf\Slide_Explanation_Bangla_Full.pdf
echo Bangla presenter notes: output\pdf\Beach_Day_Presenter_Notes_Bangla.pdf
start "" "%PROJECT_EXE%"
if errorlevel 1 goto launch_error
popd
exit /b 0

:build_complete
echo Build completed successfully.
popd
exit /b 0

:build_error
echo.
echo Build failed. See build\launcher.log for details.
echo If Project1.exe is locked, close the running project and try again.
goto failed

:tools_error
echo Install Visual Studio with Desktop development with C++,
echo the v145 toolset and Windows SDK, then try again.
goto failed

:missing_exe
echo The Release executable was not produced. Check build\launcher.log.
goto failed

:launch_error
echo Could not open the project executable.
goto failed

:process_error
echo Could not close the previous Beach Day instance. See the message above.
goto failed

:directory_error
echo Could not open the project folder.
pause
exit /b 1

:failed
popd
if /i "%~1"=="--build-only" exit /b 1
pause
exit /b 1
