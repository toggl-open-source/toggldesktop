@echo off
setlocal

rem
rem buildwin.cmd
rem
rem POCO C++ Libraries command-line build script 
rem for MS Visual Studio 2003 to 2013
rem
rem $Id: //poco/1.4/dist/buildwin.cmd#10 $
rem
rem Copyright (c) 2006-2014 by Applied Informatics Software Engineering GmbH
rem and Contributors.
rem
rem Original version by Aleksandar Fabijanic.
rem Modified by Guenter Obiltschnig.
rem
rem Usage:
rem ------
rem buildwin VS_VERSION [ACTION] [LINKMODE] [CONFIGURATION] [PLATFORM] [SAMPLES] [TOOL]
rem VS_VERSION:    71|80|90|100|110|120
rem ACTION:        build|rebuild|clean
rem LINKMODE:      static_mt|static_md|shared|all
rem CONFIGURATION: release|debug|both
rem PLATFORM:      Win32|x64|WinCE|WEC2013
rem SAMPLES:       samples|nosamples
rem TOOL:          devenv|vcexpress|msbuild
rem
rem VS_VERSION is required argument. Default is build all.

if not defined VCINSTALLDIR (
echo Error: No Visual C++ environment found.
echo Please run this script from a Visual Studio Command Prompt
echo or run "%%VSnnCOMNTOOLS%%\vcvars32.bat" first.
goto :EOF)

rem Change OPENSSL_DIR to match your setup
set OPENSSL_DIR=c:\OpenSSL
set OPENSSL_INCLUDE=%OPENSSL_DIR%\include
set OPENSSL_LIB=%OPENSSL_DIR%\lib\VC
set INCLUDE=%INCLUDE%;%OPENSSL_INCLUDE%
set LIB=%LIB%;%OPENSSL_LIB%

set POCO_BASE=%CD%
set PATH=%POCO_BASE%\bin64;%POCO_BASE%\bin;%PATH%

rem VS version {71|80|90|100|110|120}
if "%1"=="" goto usage
set VS_VERSION=vs%1

if "%7"=="" goto use_devenv
set BUILD_TOOL=%7
goto use_custom

:use_devenv
set BUILD_TOOL=devenv
if "%VS_VERSION%"=="vs100" (set BUILD_TOOL=msbuild)
if "%VS_VERSION%"=="vs110" (set BUILD_TOOL=msbuild)
if "%VS_VERSION%"=="vs120" (set BUILD_TOOL=msbuild)

:use_custom
if not "%BUILD_TOOL%"=="msbuild" (set USEENV=/useenv)
if "%BUILD_TOOL%"=="msbuild" (
set ACTIONSW=/t:
set CONFIGSW=/p:Configuration=
set EXTRASW=/m
set USEENV=/p:UseEnv=true
)
if not "%BUILD_TOOL%"=="msbuild" (set ACTIONSW=/)

if "%VS_VERSION%"=="vs100" (goto msbuildok)
if "%VS_VERSION%"=="vs110" (goto msbuildok)
if "%VS_VERSION%"=="vs120" (goto msbuildok)
if "%BUILD_TOOL%"=="msbuild" (
echo "Cannot use msbuild with Visual Studio 2008 or earlier."
exit /b 1
)
:msbuildok

rem Action [build|rebuild|clean]
set ACTION=%2
if not "%ACTION%"=="build" (
if not "%ACTION%"=="rebuild" (
if not "%ACTION%"=="" (
if not "%ACTION%"=="clean" goto usage)))

if "%ACTION%"=="" (set ACTION="build")

rem Link mode [static|shared|both]
set LINK_MODE=%3
if not "%LINK_MODE%"=="static_mt" (
if not "%LINK_MODE%"=="static_md" (
if not "%LINK_MODE%"=="shared" (
if not "%LINK_MODE%"=="" (
if not "%LINK_MODE%"=="all" goto usage))))

rem Configuration [release|debug|both]
set CONFIGURATION=%4
if not "%CONFIGURATION%"=="release" (
if not "%CONFIGURATION%"=="debug" (
if not "%CONFIGURATION%"=="" (
if not "%CONFIGURATION%"=="both" goto usage)))

rem Platform [Win32|x64|WinCE|WEC2013]
set PLATFORM=%5%
if not "%PLATFORM%"=="" (
if not "%PLATFORM%"=="Win32" (
if not "%PLATFORM%"=="x64" (
if not "%PLATFORM%"=="WinCE" (
if not "%PLATFORM%"=="WEC2013" goto usage))))

if "%PLATFORM%"=="" (set PLATFORM_SUFFIX=) else (
if "%PLATFORM%"=="Win32" (set PLATFORM_SUFFIX=) else (
if "%PLATFORM%"=="x64" (set PLATFORM_SUFFIX=_x64) else (
if "%PLATFORM%"=="WinCE" (set PLATFORM_SUFFIX=_CE) else (
if "%PLATFORM%"=="WEC2013" (set PLATFORM_SUFFIX=_WEC2013)))))

if "%PLATFORM%"=="WEC2013" (
set PLATFORMSW=/p:Platform=%WEC2013_PLATFORM%
if "%WEC2013_PLATFORM%"=="" (
echo WEC2013_PLATFORM not set. Exiting.
exit /b 1
)
set USEENV=
)

rem Samples [samples|nosamples]
set SAMPLES=%6
if "%SAMPLES%"=="" (set SAMPLES=samples)

set DEBUG_SHARED=0
set RELEASE_SHARED=0
set DEBUG_STATIC_MT=0
set RELEASE_STATIC_MT=0
set DEBUG_STATIC_MD=0
set RELEASE_STATIC_MD=0

if "%LINK_MODE%"=="shared" (
if "%CONFIGURATION%"=="release" (set RELEASE_SHARED=1) else (
if "%CONFIGURATION%"=="both" (set RELEASE_SHARED=1) else (
if "%CONFIGURATION%"=="" (set RELEASE_SHARED=1))))

if "%LINK_MODE%"=="shared" (
if "%CONFIGURATION%"=="debug" (set DEBUG_SHARED=1) else (
if "%CONFIGURATION%"=="both" (set DEBUG_SHARED=1) else (
if "%CONFIGURATION%"=="" (set DEBUG_SHARED=1))))

if "%LINK_MODE%"=="static_mt" (
if "%CONFIGURATION%"=="release" (set RELEASE_STATIC_MT=1) else (
if "%CONFIGURATION%"=="both" (set RELEASE_STATIC_MT=1) else (
if "%CONFIGURATION%"=="" (set RELEASE_STATIC_MT=1))))

if "%LINK_MODE%"=="static_md" (
if "%CONFIGURATION%"=="release" (set RELEASE_STATIC_MD=1) else (
if "%CONFIGURATION%"=="both" (set RELEASE_STATIC_MD=1) else (
if "%CONFIGURATION%"=="" (set RELEASE_STATIC_MD=1))))

if "%LINK_MODE%"=="static_mt" (
if "%CONFIGURATION%"=="debug" (set DEBUG_STATIC_MT=1) else (
if "%CONFIGURATION%"=="both" (set DEBUG_STATIC_MT=1) else (
if "%CONFIGURATION%"=="" (set DEBUG_STATIC_MT=1))))

if "%LINK_MODE%"=="static_md" (
if "%CONFIGURATION%"=="debug" (set DEBUG_STATIC_MD=1) else (
if "%CONFIGURATION%"=="both" (set DEBUG_STATIC_MD=1) else (
if "%CONFIGURATION%"=="" (set DEBUG_STATIC_MD=1))))

if "%LINK_MODE%"=="all" (
if "%CONFIGURATION%"=="debug" (
set DEBUG_STATIC_MT=1
set DEBUG_STATIC_MD=1
set DEBUG_SHARED=1) else (
if "%CONFIGURATION%"=="release" (
set RELEASE_STATIC_MT=1
set RELEASE_STATIC_MD=1
set RELEASE_SHARED=1) else (
if "%CONFIGURATION%"=="both" (
set DEBUG_STATIC_MT=1
set DEBUG_STATIC_MD=1
set DEBUG_SHARED=1
set RELEASE_STATIC_MT=1
set RELEASE_STATIC_MD=1
set RELEASE_SHARED=1) else (
if "%CONFIGURATION%"=="" (
set DEBUG_STATIC_MT=1
set DEBUG_STATIC_MD=1
set DEBUG_SHARED=1
set RELEASE_STATIC_MT=1
set RELEASE_STATIC_MD=1
set RELEASE_SHARED=1)))))

if "%LINK_MODE%"=="" (
if "%CONFIGURATION%"=="debug" (
set DEBUG_STATIC_MT=1
set DEBUG_STATIC_MD=1
set DEBUG_SHARED=1) else (
if "%CONFIGURATION%"=="release" (
set RELEASE_STATIC_MT=1
set RELEASE_STATIC_MD=1
set RELEASE_SHARED=1) else (
if "%CONFIGURATION%"=="both" (
set DEBUG_STATIC_MT=1
set DEBUG_STATIC_MD=1
set DEBUG_SHARED=1
set RELEASE_STATIC_MT=1
set RELEASE_STATIC_MD=1
set RELEASE_SHARED=1) else (
if "%CONFIGURATION%"=="" (
set DEBUG_STATIC_MT=1
set DEBUG_STATIC_MD=1
set DEBUG_SHARED=1
set RELEASE_STATIC_MT=1
set RELEASE_STATIC_MD=1
set RELEASE_SHARED=1)))))

echo.
echo.
echo ########################################################################
echo ####
echo #### STARTING VISUAL STUDIO BUILD (%VS_VERSION%, %PLATFORM%)
echo ####
echo ########################################################################
echo.
echo.
echo The following configurations will be built:

if %DEBUG_SHARED%==1      (echo debug_shared)
if %RELEASE_SHARED%==1    (echo release_shared)
if %DEBUG_STATIC_MT%==1   (echo debug_static_mt)
if %DEBUG_STATIC_MD%==1   (echo debug_static_md)
if %RELEASE_STATIC_MT%==1 (echo release_static_mt)
if %RELEASE_STATIC_MD%==1 (echo release_static_md)

rem build for up to 4 levels deep
for /f %%G in ('findstr /R "." components') do (
 if exist %%G (
  cd %%G
  for /f "tokens=1,2,3,4 delims=/" %%Q in ("%%G") do (
   if exist %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln (
    echo.
    echo.
    echo ########################################################################
    echo ####
    echo #### Building %%G
    echo ####
    echo ########################################################################
    echo.
    if %DEBUG_SHARED%==1      (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_shared %PLATFORMSW% %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_SHARED%==1    (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_shared %PLATFORMSW% %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MT%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_mt %PLATFORMSW% %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MT%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_mt %PLATFORMSW% %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MD%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_md %PLATFORMSW% %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MD%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_md %PLATFORMSW% %%Q%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
   )

   if exist %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln (
    echo.
    echo.
    echo ########################################################################
    echo ####
    echo #### Building %%G
    echo ####
    echo ########################################################################
    echo.
    if %DEBUG_SHARED%==1      (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_shared %PLATFORMSW% %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_SHARED%==1    (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_shared %PLATFORMSW% %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MT%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_mt %PLATFORMSW% %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MT%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_mt %PLATFORMSW% %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MD%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_md %PLATFORMSW% %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MD%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_md %PLATFORMSW% %%R%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
   )

   if exist %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln (
    echo.
    echo.
    echo ########################################################################
    echo ####
    echo #### Building %%G
    echo ####
    echo ########################################################################
    echo.
    if %DEBUG_SHARED%==1      (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_shared %PLATFORMSW% %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_SHARED%==1    (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_shared %PLATFORMSW% %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MT%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_mt %PLATFORMSW% %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MT%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_mt %PLATFORMSW% %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MD%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_md %PLATFORMSW% %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MD%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_md %PLATFORMSW% %%S%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
   )

   if exist %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln (
    echo.
    echo.
    echo ########################################################################
    echo ####
    echo #### Building %%G
    echo ####
    echo ########################################################################
    echo.
    if %DEBUG_SHARED%==1      (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_shared %PLATFORMSW% %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_SHARED%==1    (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_shared %PLATFORMSW% %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MT%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_mt %PLATFORMSW% %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MT%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_mt %PLATFORMSW% %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %DEBUG_STATIC_MD%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_md %PLATFORMSW% %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
    if %RELEASE_STATIC_MD%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_md %PLATFORMSW% %%T%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
   )
  )
  cd %POCO_BASE%
 )
)

if "%SAMPLES%"=="no" goto :EOF
if "%SAMPLES%"=="nosamples" goto :EOF

rem root level component samples
for /f %%G in ('findstr /R "." components') do (
 if exist %%G\samples\samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln (
  cd %%G\samples
    echo.
    echo.
    echo ########################################################################
    echo ####
    echo #### Building %%G/samples
    echo ####
    echo ########################################################################
    echo.
  if %DEBUG_SHARED%==1      (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_shared %PLATFORMSW% samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
  if %RELEASE_SHARED%==1    (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_shared %PLATFORMSW% samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
  if %DEBUG_STATIC_MT%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_mt %PLATFORMSW% samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
  if %RELEASE_STATIC_MT%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_mt %PLATFORMSW% samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
  if %DEBUG_STATIC_MD%==1   (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%debug_static_md %PLATFORMSW% samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
  if %RELEASE_STATIC_MD%==1 (%BUILD_TOOL% %USEENV% %EXTRASW% %ACTIONSW%%ACTION% %CONFIGSW%release_static_md %PLATFORMSW% samples%PLATFORM_SUFFIX%_%VS_VERSION%.sln && echo. && echo. && echo.)
  cd %POCO_BASE%
 )
)

goto :EOF

:usage
echo Usage:
echo ------
echo buildwin VS_VERSION [ACTION] [LINKMODE] [CONFIGURATION] [PLATFORM] [SAMPLES] [TOOL]
echo VS_VERSION:    "71|80|90|100|110|120"
echo ACTION:        "build|rebuild|clean"
echo LINKMODE:      "static_mt|static_md|shared|all"
echo CONFIGURATION: "release|debug|both"
echo PLATFORM:      "Win32|x64|WinCE|WEC2013"
echo SAMPLES:       "samples|nosamples"
echo TOOL:          "devenv|vcexpress|msbuild"
echo.
echo Default is build all.
endlocal
