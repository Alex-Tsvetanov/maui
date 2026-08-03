@echo off
title Add MSVC ARM64 build tools (for the maui C++ port)
color 0B

echo ================================================================
echo   Add the MSVC ARM64 TARGET compiler to VS Build Tools 2022
echo ================================================================
echo.
echo WHY: this guest currently has only x86/x64 target compilers
echo      (Hostarm64\x64, Hostarm64\x86 - no arm64 target at all).
echo      The MAUI reference app was built NATIVE win-arm64, so the
echo      C++ port must also build arm64 or the parity comparison
echo      would measure two different rendering paths.
echo.
echo Three silent/automated attempts already failed:
echo      vs_installer modify        -^> exit 87
echo      winget --override         -^> argument rejected
echo      vs_installer.windows.exe  -^> exit -1
echo   ...which is why this needs a human at the GUI.
echo.
echo WHAT WILL HAPPEN: the Visual Studio Installer opens with the
echo   ARM64 component pre-selected. Click "Modify" to confirm.
echo.
echo IF THE PRE-SELECTION DOES NOT TAKE, do it by hand:
echo   Individual components -^> search: ARM64
echo.
echo   TICK THIS ONE (note the "64" and the ARM64EC part):
echo       MSVC v143 - VS 2022 C++ ARM64/ARM64EC build tools (Latest)
echo.
echo   NOT this one - it is 32-bit ARM and does NOT help:
echo       MSVC v143 - VS 2022 C++ ARM build tools (Latest)
echo.
echo   (A previous run installed the plain "ARM" one by mistake, which
echo    is why this script now spells out the difference.)
echo.
echo ================================================================
pause

set "VSI=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
set "VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"

if not exist "%VSI%" (
  echo.
  echo ERROR: Visual Studio Installer not found at:
  echo   %VSI%
  echo.
  pause
  exit /b 1
)

echo.
echo Launching the installer...
echo.

REM --passive shows progress UI and needs no further clicks; deliberately NOT --quiet,
REM because the silent modes are what swallowed the error in the failed attempts.
start "" /wait "%VSI%" modify --installPath "%VSPATH%" --add Microsoft.VisualStudio.Component.VC.Tools.ARM64 --passive --norestart

echo.
echo Installer finished (exit code %ERRORLEVEL%).
echo.
echo Verifying whether an arm64 target compiler now exists...
echo.

powershell -NoProfile -Command ^
  "$p='C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC';" ^
  "$t=Get-ChildItem $p -Recurse -Filter cl.exe -ErrorAction SilentlyContinue |" ^
  "   ForEach-Object { $_.Directory.Parent.Name + '/' + $_.Directory.Name } | Sort-Object -Unique;" ^
  "Write-Host ('targets: ' + ($t -join ', '));" ^
  "if ($t -contains 'Hostarm64/arm64') { Write-Host 'SUCCESS: native arm64 compiler is present.' -ForegroundColor Green }" ^
  "else { Write-Host 'NOT YET: no Hostarm64/arm64 - re-run and tick the component by hand.' -ForegroundColor Yellow }"

echo.
echo Tell Claude when this is done and it will re-verify from the host.
echo.
pause
