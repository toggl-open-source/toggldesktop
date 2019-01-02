# This script will download and build OpenSSL in debug, release
# or both configurations for Win32 or x64.
#
# Requires PowerShell version 3.0 or newer
#
# Usage:
# ------
# build.ps1 [-openssl_release 1.0.0 | 1.1.0]
#           [-vs_version 150 | 140 | 120 | 110 | 100 | 90]
#           [-config     release | debug | both]
#           [-platform   Win32 | x64]
#           [-library    shared | static | both]
#
# Example: C:\>powershell -ExecutionPolicy RemoteSigned \
#          -File build.ps1 -openssl_release 1.1.0 -vs_version 120 \
#          -config both -platform x64 -library both
#
# Modified and adapted for POCO from https://github.com/hadouken/openssl
# (see https://github.com/hadouken/openssl/blob/master/LICENSE for license)
#

[CmdletBinding()]
Param
(
  [Parameter()]
  [ValidatePattern("1\.[0-9]\.[0-9][a-z]*")]
  [string] $openssl_release = "1.1.0",

  [Parameter()]
  [ValidateSet(90, 100, 110, 120, 140, 150)]
  [int] $vs_version = 120,

  [Parameter()]
  [ValidateSet('release', 'debug', 'both')]
  [string] $config = 'release',

  [Parameter()]
  [ValidateSet('Win32', 'x64')]
  [string] $platform = 'x64',

  [Parameter()]
  [ValidateSet('shared', 'static', 'both')]
  [string] $library = 'shared'
)

$configstr = $config
if ($config -eq 'both') { $configstr = "debug and release"}
$librarystr = $library
if ($library -eq 'both') { $librarystr = "shared and static"}
Write-Host "Building VS$vs_version ($platform), Config: $configstr, Library: $librarystr"
Write-host "Please wait, this may take some time ..."

$BUILD_ROOT = Join-Path $PSScriptRoot "VS_$vs_version"
$PACKAGES_DIRECTORY   = Join-Path $PSScriptRoot "packages"
$OUTPUT_BIN_DIRECTORY = $BUILD_ROOT
$OUTPUT_INC_DIRECTORY = Join-Path $BUILD_ROOT "include"
$VERSION              = "0.0.0"

if (Test-Path Env:\APPVEYOR_BUILD_VERSION) {
    $VERSION = $env:APPVEYOR_BUILD_VERSION
}

# 7zip configuration section
$7ZIP_VERSION      = "18.01"
$7ZIPNODOTVERSION  = $7ZIP_VERSION -replace "\.", ""
$7ZIP_NAME         = "7z-$7ZIPNODOTVERSION-x64"
$7ZIP_DIRECTORY    = Join-Path $PACKAGES_DIRECTORY $7ZIP_NAME
$7ZIP_TOOL         = Join-Path $7ZIP_DIRECTORY "\Files\7-Zip\7z.exe"
$7ZIP_PACKAGE_FILE = "7z$7ZIPNODOTVERSION.msi"
$7ZIPINSTALLER     = "$PACKAGES_DIRECTORY\$7ZIP_PACKAGE_FILE"
$7ZIP_DOWNLOAD_URL = "http://7-zip.org/a/$7ZIP_PACKAGE_FILE"

# NASM configuration section
$NASM_VERSION      = "2.13"
$NASM_DIRECTORY    = Join-Path $PACKAGES_DIRECTORY "nasm-$NASM_VERSION"
$NASM_PACKAGE_FILE = "nasm-$NASM_VERSION-win32.zip"
$NASM_DOWNLOAD_URL = "http://www.nasm.us/pub/nasm/releasebuilds/$NASM_VERSION/win32/$NASM_PACKAGE_FILE"

# Strawberry Perl configuration section
#$PERL_VERSION      = "5.20.1.1"
$PERL_VERSION      = "5.26.1.1"
$PERL_DIRECTORY    = Join-Path $PACKAGES_DIRECTORY "strawberry-perl-$PERL_VERSION"
$PERL_PACKAGE_FILE = "strawberry-perl-$PERL_VERSION-32bit-portable.zip"
$PERL_DOWNLOAD_URL = "http://strawberryperl.com/download/$PERL_VERSION/$PERL_PACKAGE_FILE"

# OpenSSL configuration section
$OPENSSL_VERSION         = $openssl_release
$ver_arr = $OPENSSL_VERSION.Split(".")
$OPENSSL_MAJOR_VERSION = $ver_arr[0]
$OPENSSL_MINOR_VERSION = $ver_arr[1]
$OPENSSL_DIRECTORY       = Join-Path $PACKAGES_DIRECTORY "openssl-$OPENSSL_VERSION"
$OPENSSL_CLEAN_DIRECTORY = Join-Path $PACKAGES_DIRECTORY "openssl-$OPENSSL_VERSION.clean"
$OPENSSL_PACKAGE_FILE    = "openssl-$OPENSSL_VERSION.tar.gz"
$OPENSSL_DOWNLOAD_URL    = "https://www.openssl.org/source/$OPENSSL_PACKAGE_FILE"


function Download-File {
    param (
        [string]$url,
        [string]$target
    )

  $webClient = new-object System.Net.WebClient
  $webClient.DownloadFile($url, $target)
}


function Extract-File {
    param (
      [string]$file,
      [string]$target
    )
  if (Test-Path -Path $7ZIP_TOOL) { & "$7ZIP_TOOL" x $file -o"$target" -y }
  else {
    # first time, no 7Zip yet, install
    $cmd = "cmd /c start /wait msiexec"
    $target = "TARGETDIR=$7ZIP_DIRECTORY"
    $log = "/log $PACKAGES_DIRECTORY\7zip_install.log"
    $exp = "$cmd /a $7ZIPINSTALLER /quiet /qn /norestart $target $log"
    Write-Host "Installing 7Zip: $exp"
    Invoke-Expression $exp
  }
}


function Load-DevelopmentTools {
  # Set environment variables for Visual Studio Command Prompt

  if ($vs_version -eq 0)
    {
      if     ($Env:VS150COMNTOOLS -ne '') { $script:vs_version = 150 }
      elseif ($Env:VS140COMNTOOLS -ne '') { $script:vs_version = 140 }
      elseif ($Env:VS120COMNTOOLS -ne '') { $script:vs_version = 120 }
      elseif ($Env:VS110COMNTOOLS -ne '') { $script:vs_version = 110 }
      elseif ($Env:VS100COMNTOOLS -ne '') { $script:vs_version = 100 }
      elseif ($Env:VS90COMNTOOLS  -ne '') { $script:vs_version = 90 }
      else
      {
        Write-Host 'Visual Studio not found, exiting.'
        Exit
      }
    }

    $vsct = "VS$($vs_version)COMNTOOLS"
    $vsdir = (Get-Item Env:$vsct).Value
    $Command = ''

    if ($vs_version -ge 150)
    {
      if ($platform -eq 'x64')
      {
        $Command = "$($vsdir)\..\..\VC\Auxiliary\Build\vcvars64.bat"
      }
      else
      {
        $Command = "$($vsdir)\..\..\VC\Auxiliary\Build\vcvars32.bat"
      }
    }
    else
    {
      if ($platform -eq 'x64')
      {
        $Command = "$($vsdir)..\..\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
      }
      else
      {
        $Command = "$($vsdir)vsvars32.bat"
      }
    }
    $tempFile = [IO.Path]::GetTempFileName()
    cmd /c " `"$Command`" && set > `"$tempFile`" "
    Get-Content $tempFile | Foreach-Object {
      if($_ -match "^(.*?)=(.*)$")
      {
        Set-Content "Env:$($matches[1])" $matches[2]
      }
    }
    Remove-Item $tempFile
}


function Download-Package {
    param (
      [string]$url,
      [string]$file
    )

  $dir = $PACKAGES_DIRECTORY

  if (!(Test-Path $dir)) {
    New-Item -ItemType Directory -Path $dir | Out-Null
  }

  if (!(Test-Path (Join-Path $dir $file))) {
    Write-Host "Downloading $file from $url"
    Download-File $url (Join-Path $dir $file)
  }
}


function Download {

  # Download 7zip
  Download-Package $7ZIP_DOWNLOAD_URL $7ZIP_PACKAGE_FILE

  # Download NASM
  Download-Package $NASM_DOWNLOAD_URL $NASM_PACKAGE_FILE

  # Download Strawberry-Perl
  Download-Package $PERL_DOWNLOAD_URL $PERL_PACKAGE_FILE

  # Download OpenSSL
  Download-Package $OPENSSL_DOWNLOAD_URL $OPENSSL_PACKAGE_FILE
}


function Unzip-Package {
    param (
      [string]$dir,
      [string]$file
    )

  $pdir = $PACKAGES_DIRECTORY
  if (!(Test-Path $dir)) {
    Write-Host "Unpacking $file"
    Extract-File (Join-Path $pdir $file) $dir
  }
}


function Unzip {
    Unzip-Package $7ZIP_DIRECTORY $7ZIP_PACKAGE_FILE
    Unzip-Package $PERL_DIRECTORY $PERL_PACKAGE_FILE

  # Unpack NASM separately, package has nested dir layout
  if (!(Test-Path $NASM_DIRECTORY)) {
    Write-Host "Unpacking $NASM_PACKAGE_FILE"
    Extract-File (Join-Path $PACKAGES_DIRECTORY $NASM_PACKAGE_FILE) $PACKAGES_DIRECTORY
  }
}


function Unpack-OpenSSL
{
  if (!(Test-Path -Path $OPENSSL_CLEAN_DIRECTORY -ErrorAction SilentlyContinue)) {

    # Unpack OpenSSL
    Write-Host "Unpacking $OPENSSL_PACKAGE_FILE"
    $tmp = Join-Path $PACKAGES_DIRECTORY $OPENSSL_PACKAGE_FILE

    & "$7ZIP_TOOL" x $tmp -o"$PACKAGES_DIRECTORY" -y
    if ($LastExitCode -ne 0) {
        Write-Host "Error extracting files"
        Exit 1
    }
    $tmp = $tmp.replace('.gz', '')
    & "$7ZIP_TOOL" x $tmp -o"$PACKAGES_DIRECTORY" -y
    if ($LastExitCode -ne 0) {
        Write-Host "Error extracting files"
        Exit 1
    }

    # This is a clean code base that will be copied and used by each individual build
    Start-Sleep 1
    Rename-Item -path $OPENSSL_DIRECTORY -newname "$OPENSSL_CLEAN_DIRECTORY" -force
    if ($? -eq $false) {
        Write-Host "Error renaming OpenSSL directory [$OPENSSL_DIRECTORY] to [$OPENSSL_CLEAN_DIRECTORY]"
        Exit 1
    }
  }

  # Clean up stale files from the previous build
  Start-Sleep 1
  if (Test-Path -Path $OPENSSL_DIRECTORY -ErrorAction SilentlyContinue) {
      Remove-Item $OPENSSL_DIRECTORY -Recurse -Force
      if ($? -eq $false) {
          Write-Host "Error removing old OpenSSL directory: $OPENSSL_DIRECTORY"
          Exit 1
      }
  }

  # Copy clean directory tree for this build
  Start-Sleep 1
  Copy-Item -Path "$OPENSSL_CLEAN_DIRECTORY" -Destination "$OPENSSL_DIRECTORY" -Recurse -Force
  if ($? -eq $false) {
      Write-Host "Error copying clean OpenSSL directory [$OPENSSL_CLEAN_DIRECTORY] to [$OPENSSL_DIRECTORY]"
      Exit 1
  }
}


function Replace-String
{
  param (
      [string]$file,
      [string]$original,
      [string]$replacement
  )
  (Get-Content $file).replace($original, $replacement) | Set-Content $file
}


function Compile-OpenSSL-10
{
  param (
      [string]$winplatform,
      [string]$configuration,
      [string]$target,
      [string]$library
  )

  # start every build with clean directory
  Unpack-OpenSSL
  Start-Sleep 1
  pushd $OPENSSL_DIRECTORY

  # Set up portable Strawberry Perl
  $env:Path = "$(Join-Path $PERL_DIRECTORY perl\site\bin);" + $env:Path
  $env:Path = "$(Join-Path $PERL_DIRECTORY perl\bin);" + $env:Path
  $env:Path = "$(Join-Path $PERL_DIRECTORY c\bin);" + $env:Path

  $d = ""
  if ($configuration -eq "debug") { $d = "d" }

  $l = ""
  if ($library -eq "shared") { $l = "MD" }
  else                       { $l = "MT" }

  $b = "32"
  if ($winplatform -eq "win64") { $b = "64" }

  # Set up nasm
  $env:Path = "$NASM_DIRECTORY;" + $env:Path

  perl Configure $target --prefix="$OPENSSL_DIRECTORY/bin/$winplatform/$configuration"

  # Run nasm
  cmd /c ms\do_nasm.bat

  if ($winplatform -eq "win64") {
    # do not edit these replaces - they must be exactly like this, tabs and all
    Replace-String "util\pl\VC-32.pl" "`$ssl=	`"ssleay32`"" `
                                      "`$ssl=	`"libssl`""

    Replace-String "util\pl\VC-32.pl" "`$crypto=`"libeay32`"" `
                                      "`$crypto=`"libcrypto`""

    Replace-String "util\pl\VC-32.pl" "`$crypto=`"libeayfips32`"" `
                                      "`$crypto=`"libcryptofips`""

    Replace-String "ms\do_win64a.bat" "perl util\mkdef.pl 32 libeay > ms\libeay32.def" `
                                      "perl util\mkdef.pl $b libeay > ms\libcrypto.def"

    Replace-String "ms\do_win64a.bat" "perl util\mkdef.pl 32 ssleay > ms\ssleay32.def" `
                                      "perl util\mkdef.pl $b ssleay > ms\libssl.def"

    cmd /c ms\do_win64a

    Replace-String "ms\libcrypto.def" "LIBEAY32" "LIBCRYPTO"
    Replace-String "ms\libssl.def" "SSLEAY32" "LIBSSL"
  }
  else {
    Replace-String "ms\libeay32.def" "LIBEAY32" "LIBCRYPTO"
    Replace-String "ms\ssleay32.def" "SSLEAY32" "LIBSSL"
  }

  $lib = ""
  if ($library -eq "shared") { $lib = "dll" }
  if ($configuration -eq "debug") {
    Replace-String "ms\nt$lib.mak" "`$(TMP_D)/lib" "`$(TMP_D)/openssl"
  }

  # Run nmake
  nmake -f ms\nt$lib.mak

  # Run nmake install
  nmake -f ms\nt$lib.mak install

  popd
}


function Replace-Line
{
    param (
        [string]$file,
        [string]$regex,
        [string]$replacement
    )

    (Get-Content $file) | Foreach-Object {$_ -replace "^$regex.*", "$replacement"} | Set-Content $file
}


function Get-Suffix
{
    param (
        [string]$configuration,
        [string]$library
    )

    $suffix = ""
    if ($library -eq "static") { $suffix = "mt" }
    if ($configuration -eq "debug") { $suffix = "$($suffix)d" }

    return $suffix
}


function Compile-OpenSSL-11
{
  param (
      [string]$winplatform,
      [string]$configuration,
      [string]$target,
      [string]$library
  )

  # start every build with clean directory
  Unpack-OpenSSL
  Start-Sleep 1
  pushd $OPENSSL_DIRECTORY

  # Set up portable Strawberry Perl
  $env:Path = "$(Join-Path $PERL_DIRECTORY perl\site\bin);" + $env:Path
  $env:Path = "$(Join-Path $PERL_DIRECTORY perl\bin);" + $env:Path
  $env:Path = "$(Join-Path $PERL_DIRECTORY c\bin);" + $env:Path

  $lnk = "shared"
  $opts = ""
  if ($library -eq "static") { $lnk = "no-shared" }

  # Set up nasm
  $env:Path = "$NASM_DIRECTORY;" + $env:Path

  if ($library -eq "shared") {
    # edit build.info, to replace default name decoration for DLLS
    $suffix = Get-Suffix $configuration $library

    $replace = " SHARED_NAME[libcrypto]=libcrypto"
    $regex = [Regex]::Escape($replace)
    Replace-Line "build.info" "^$regex.*" "$($replace)$($suffix)"

    $replace = " SHARED_NAME[libssl]=libssl"
    $regex = [Regex]::Escape($replace)
    Replace-Line "build.info" "^$regex.*" "$($replace)$($suffix)"
  }

  $openssldir = "$OPENSSL_DIRECTORY\ssl"
  $prefix = "$OPENSSL_DIRECTORY\bin\$winplatform\$configuration"
  $cmd = "perl Configure $target --api=1.0.0 $opts --prefix=$prefix --openssldir=$openssldir $lnk"

  Write-Host $cmd
  Invoke-Expression $cmd

  nmake
  nmake install_sw

  # ugly hack - we want all versions of VS to be able to link with VS 120 libs
  # older versions CRT libs miss some functions (related to SSE2), so we extract
  # the object containing those functions and put it in a separate library
  # (only needed for 32-bit static libs, built with /MT)
  if (($script:vs_version -eq 120) -and ($library -eq "static")) {
    $crtlib = "msvcrt.lib"
    if ($configuration -eq "debug") { $crtlib = "msvcrtd.lib" }
    $ftolObjPath = Invoke-Expression "lib /NOLOGO /LIST msvcrt.lib | findstr ftol3 2>&1"
    Write-Host "ftolObjPath :[$ftolObjPath]"
    if ($ftolObjPath) {
      Write-Host "Found $ftolObjPath"
      $ftolOut = Split-Path $ftolObjPath -leaf
      if ($ftolOut) {
        Write-Host "Found $ftolOut"
        $extFtol = "$prefix\$ftolOut"
        Write-Host "Extracting $ftolObjPath to $extFtol"
        Invoke-Expression "lib /NOLOGO /EXTRACT:$ftolObjPath /OUT:$extFtol msvcrt.lib"
        $suffix = Get-Suffix $configuration $library
        $libPreVS2013CRT = "$prefix\lib\libPreVS2013CRT$($suffix).lib"
        Write-Host "Creating $libPreVS2013CRT from $extFtol"
        Invoke-Expression "lib /NOLOGO /OUT:$libPreVS2013CRT $extFtol"
      }
    }
  }

  popd
}


function Compile-OpenSSL
{
  param (
      [string]$winplatform,
      [string]$configuration,
      [string]$target,
      [string]$library
  )

  Write-Host "**********************************************************"
  Write-Host " Compiling $winplatform $configuration $target $library"
  Write-Host "**********************************************************"

  if (($OPENSSL_MAJOR_VERSION -eq 1) -and ($OPENSSL_MINOR_VERSION -eq 0)) {
      Compile-OpenSSL-10 $winplatform $configuration $target $library
  }
  elseif (($OPENSSL_MAJOR_VERSION -ge 1) -and ($OPENSSL_MINOR_VERSION -ge 1)) {
      Compile-OpenSSL-11 $winplatform $configuration $target $library
  }
  else {
      throw "Only versions >= 1.0.x are supported"
  }
}


function Output-OpenSSL-10
{
  param (
      [string]$winplatform,
      [string]$configuration,
      [string]$library
  )

  pushd $OPENSSL_DIRECTORY

  $t = Join-Path $OUTPUT_BIN_DIRECTORY "$winplatform"
  $lib = "lib"

  $d = ""
  if ($configuration -eq "debug") { $d = "d" }

  $l = ""
  if ($library -eq "shared") { $l = "MD" }
  else                       { $l = "MT" }

  $b = "32"
  if ($winplatform -eq "win64") { $b = "64" }

  # Copy output files
  if ($library -eq "shared") {
      xcopy /y bin\$winplatform\$configuration\bin\*.dll "$t\bin\$configuration\*"

      if ($winplatform -eq "win32") {
          $path = "$t\bin\$configuration\libcrypto.dll"
          if (Test-Path $path) { Remove-Item $path -force }
          $path = "$t\bin\$configuration\libeay32.dll"
          Rename-Item -path $path -newname "libcrypto.dll" -force

          $path = "$t\bin\$configuration\libssl.dll"
          if (Test-Path $path) { Remove-Item $path -force }
          $path = "$t\bin\$configuration\ssleay32.dll"
          Rename-Item -path $path -newname "libssl.dll" -force
      }

      $lib = "bin"
  }

  xcopy /y bin\$winplatform\$configuration\lib\*.lib "$t\$lib\$configuration\*" 

  if ($configuration -eq "debug") {
      $tmp = Join-Path $OPENSSL_DIRECTORY "tmp32.dbg"
      if (Test-Path "$tmp\openssl.pdb") {
          xcopy /y "$tmp\openssl.pdb" "$t\$lib\$configuration\*" /E
      }
  }

  if ($winplatform -eq "win32") {
      $path = "$t\$lib\$configuration\libcrypto.lib"
      if (Test-Path $path) { Remove-Item $path -force }
      $path = "$t\$lib\$configuration\libeay32.lib"
      Rename-Item -path $path -newname "libcrypto.lib" -force

      $path = "$t\$lib\$configuration\libssl.lib"
      if (Test-Path $path) { Remove-Item $path -force }
      $path = "$t\$lib\$configuration\ssleay32.lib"
      Rename-Item -path $path -newname "libssl.lib" -force
  }

  if (!(Test-Path -Path "$OUTPUT_INC_DIRECTORY")) {
      xcopy /y bin\$winplatform\$configuration\include\* "$OUTPUT_INC_DIRECTORY\*" /E
  }

  popd
}


function Do-XCopy
{
  param (
      [string]$source,
      [string]$target
  )
  xcopy /y "$source" "$target" /E /C
}


function Output-OpenSSL-11 {
  param (
      [string]$winplatform,
      [string]$configuration,
      [string]$library
  )

  pushd $OPENSSL_DIRECTORY

  $t = Join-Path $OUTPUT_BIN_DIRECTORY "$winplatform"

  # Copy output files
  $lib = "lib"
  $srcpath = "bin\$winplatform\$configuration\$lib"
  $destpath = "$t\$lib\$configuration"
  if ($library -eq "shared") {
      $lib = "bin"
      $destpath = "$t\$lib\$configuration"
      Do-XCopy "$srcpath\*.lib" "$destpath\*"
      $srcpath = "bin\$winplatform\$configuration\$lib"
      Do-XCopy "$srcpath\*.dll" "$destpath\*"
      Do-XCopy "$srcpath\*.pdb" "$destpath\*"
  }
  else {
      Do-XCopy "$srcpath\*.lib" "$destpath\*"
      Do-XCopy "$srcpath\*.pdb" "$destpath\*"
  }

  $suffix = Get-Suffix $configuration $library
  if ($suffix) {
    Write-Host "Renaming $configuration $library libraries, adding suffix: $suffix"
    Move-Item -Path "$destpath\libcrypto.lib" -Destination "$destpath\libcrypto$($suffix).lib" -Force
    Move-Item -Path "$destpath\libssl.lib" -Destination "$destpath\libssl$($suffix).lib" -Force

    if ((Test-Path -Path "$destpath\libcrypto.pdb")) {
        Move-Item -Path "$destpath\libcrypto.pdb" -Destination "$destpath\libcrypto$($suffix).pdb" -Force
    }

    if ((Test-Path -Path "$destpath\libssl.pdb")) {
        Move-Item -Path "$destpath\libssl.pdb" -Destination "$destpath\libssl$($suffix).pdb" -Force
    }
  }

  $path = "$destpath\openssl.*"
  if (Test-Path $path) { Remove-Item $path -force }

  if (!(Test-Path -Path "$OUTPUT_INC_DIRECTORY")) {
      New-Item -ItemType Directory -Force -Path $OUTPUT_INC_DIRECTORY
  }

  # copy headers
  Do-XCopy "bin\$winplatform\$configuration\include\*" "$OUTPUT_INC_DIRECTORY\*"

  # add info for poco to know it is using its own openssl
  # and which VS version it was built with

  # first, let's make sure we don't do this multiple times
  $regex = [Regex]::Escape("POCO_OPENSSLV_H_INCLUDED")
  $found = $false
  Get-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h | ForEach { if ($_ -match "^[ -~]*$regex[ -~]*") {
    $found = $true
    break
  }}

  # clean file, stamp it
  if (!$found) {
      Add-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h "`n/* begin POCO OpenSSL script generated, do not edit manually */`n"
      Add-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h "#ifndef POCO_OPENSSLV_H_INCLUDED`n"
      Add-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h "  #define POCO_OPENSSLV_H_INCLUDED`n"
      Add-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h "  #define POCO_INTERNAL_OPENSSL_MSVC_VER $vs_version`n"
      Add-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h "#endif // POCO_OPENSSLV_H_INCLUDED`n"
      Add-Content $OUTPUT_INC_DIRECTORY\openssl\opensslv.h "/* end POCO OpenSSL script generated */`n"
  }

  $applinkSource = "ms\applink.c"
  $applinkTarget = "$OUTPUT_INC_DIRECTORY\openssl\applink.c"
  if ((Test-Path -Path $applinkSource) -and !(Test-Path -Path $applinkTarget)) {
      Copy-Item $applinkSource -Destination $applinkTarget
  }

  popd
}


function Output-OpenSSL {
  param (
      [string]$winplatform,
      [string]$configuration,
      [string]$library
  )
  Write-Host "************************************************"
  Write-Host " Copying $winplatform $configuration $library"
  Write-Host "************************************************"

  if (($OPENSSL_MAJOR_VERSION -eq 1) -and ($OPENSSL_MINOR_VERSION -eq 0)) {
      Output-OpenSSL-10 $winplatform $configuration $library
  }
  elseif (($OPENSSL_MAJOR_VERSION -ge 1) -and ($OPENSSL_MINOR_VERSION -ge 1)) {
      Output-OpenSSL-11 $winplatform $configuration $library
  }
  else {
      throw "Only versions >= 1.0.x are supported"
  }
}


function Copy-To-Build
{
  # (re)create universal build directory where every version build ends up
  if (!(Test-Path -PathType Container -Path $PSScriptRoot\build)) {
    New-Item -ItemType Directory -Path "$PSScriptRoot\build" -Force
  }

  Do-XCopy "$BUILD_ROOT\*" "$PSScriptRoot\build\*" -Force
  # write VS version
  Set-Content -Path "$PSScriptRoot\build\vs_version" -Value "$vs_version"
}


function Build-Win32
{
  if (($config -eq "debug") -or ($config -eq "both")) {
    if (($library -eq "shared") -or ($library -eq "both")) {
        Compile-OpenSSL "win32" "debug" "debug-VC-WIN32" "shared"
        Output-OpenSSL  "win32" "debug" "shared"
    }
    
    if (($library -eq "static") -or ($library -eq "both")) {
        Compile-OpenSSL "win32" "debug" "debug-VC-WIN32" "static"
        Output-OpenSSL  "win32" "debug" "static"
    }
  }

  if (($config -eq "release") -or ($config -eq "both")) {
    if (($library -eq "shared") -or ($library -eq "both")) {
        Compile-OpenSSL "win32" "release" "VC-WIN32" "shared"
        Output-OpenSSL  "win32" "release" "shared"
    }
    
    if (($library -eq "static") -or ($library -eq "both")) {
        Compile-OpenSSL "win32" "release" "VC-WIN32" "static"
        Output-OpenSSL  "win32" "release" "static"
    }
  }
}


function Build-Win64
{
  if (($config -eq "debug") -or ($config -eq "both")) {
    if (($library -eq "shared") -or ($library -eq "both")) {
        Compile-OpenSSL "win64" "debug" "debug-VC-WIN64A" "shared"
        Output-OpenSSL  "win64" "debug" "shared"
    }
    
    if (($library -eq "static") -or ($library -eq "both")) {
        Compile-OpenSSL "win64" "debug" "debug-VC-WIN64A" "static"
        Output-OpenSSL  "win64" "debug" "static"
    }
  }

  if (($config -eq "release") -or ($config -eq "both")) {
    if (($library -eq "shared") -or ($library -eq "both")) {
        Compile-OpenSSL "win64" "release" "VC-WIN64A" "shared"
        Output-OpenSSL  "win64" "release" "shared"
    }
    
    if (($library -eq "static") -or ($library -eq "both")) {
        Compile-OpenSSL "win64" "release" "VC-WIN64A" "static"
        Output-OpenSSL  "win64" "release" "static"
    }
  }
}


function Main
{
  Download
  Unzip

  Load-DevelopmentTools

  if ($platform -eq "Win32")   { Build-Win32 }
  elseif ($platform -eq "x64") { Build-Win64 }
  else {
    Write-Error "Unknown platform: $platform, allowed values: [Win32 | x64]"
    Exit 1
  }

  Copy-To-Build
}

Main
