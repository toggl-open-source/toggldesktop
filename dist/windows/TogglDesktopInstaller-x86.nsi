
;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"
  
!ifdef INNER
  !echo "Inner invocation"                  ; just to see what's going on
  OutFile "$%TEMP%\tempinstaller.exe"       ; not really important where this is
  SetCompress off                           ; for speed
!else
  !echo "Outer invocation"
 
  ; Call makensis again against current file, defining INNER.  This writes an installer for us which, when
  ; it is invoked, will just write the uninstaller to some location, and then exit.
 
  !execute 'makensis /DINNER "${__FILE__}"' = 0
 
  ; So now run that installer we just created as %TEMP%\tempinstaller.exe.  Since it
  ; calls quit the return value isn't zero.
 
  !system "$%TEMP%\tempinstaller.exe" = 2
 
  ; That will have written an uninstaller binary for us.  Now we sign it with your
  ; favorite code signing tool.
 
  !system '"C:\Program Files (x86)\Windows Kits\10\Tools\bin\i386\signtool.exe" sign -a -t "http://timestamp.verisign.com/scripts/timestamp.dll" -f "Certificate.pfx" "$%TEMP%\Uninstall.exe"' = 0
 
  ; Good.  Now we can carry on writing the real installer.
 
  OutFile "TogglDesktopInstaller.exe"
  SetCompressor /SOLID lzma
!endif

;Include FileFunc for GetParameters
  !include "FileFunc.nsh"

;Include LogicLib for if statements
  !include 'LogicLib.nsh'

;Include nsDialogs for custom Uninstaller page
  !include nsDialogs.nsh

;--------------------------------
;Add Macros

  !insertmacro GetParameters
  !insertmacro GetOptions

;--------------------------------
;Global variables

  Var keyLength
  Var isUpdater
  Var fromOldVersion
  Var deleteData
  Var CHECKBOX
  Var cmdLineParams

;--------------------------------
;General

  Name "Toggl Desktop"
  Icon "..\..\src\ui\windows\TogglDesktop\TogglDesktop\Resources\toggl.ico"

  ;Default installation folder. Local app data does not need
  ;admin privileges to install.
  InstallDir "$LOCALAPPDATA\TogglDesktop"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\TogglDesktop" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------

;Icons

  !define MUI_ICON "..\..\src\ui\windows\TogglDesktop\TogglDesktop\Resources\toggl.ico"

;Header image

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "toggl_nsis_header.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "toggl_nsis_header.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP_STRETCH "FitControl"

;Wizard images

  !define MUI_WELCOMEFINISHPAGE_BITMAP "toggl_nsis_image.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "toggl_nsis_image.bmp"

;Run App after install

  !define MUI_FINISHPAGE_RUN "$INSTDIR\TogglDesktop.exe"
  !define MUI_FINISHPAGE_RUN_TEXT "Launch Toggl Desktop"

;Install Location page
  !define MUI_PAGE_HEADER_TEXT "Install location"
  !define MUI_PAGE_HEADER_SUBTEXT ""
  !define MUI_DIRECTORYPAGE_TEXT_TOP "Setup will install Toggl Desktop into $INSTDIR. Click Install to start the installation"

;Run App on Windows login
  !define MUI_FINISHPAGE_SHOWREADME
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "Run Toggl Desktop on Windows login"
  !define MUI_FINISHPAGE_SHOWREADME_FUNCTION runOnStartup

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW DisableInstallPathEdit
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  UninstPage custom un.customPage
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

!define srcdir "..\..\src\ui\windows\TogglDesktop\TogglDesktop\bin\Release"
!define redist "..\..\third_party\vs_redist"

Section

  SetOutPath "$INSTDIR"

  ;Check if Old version of the app is still running and close it
  DetailPrint "Closing all old TogglDesktop processes"
  File "NSIS_plugins\KillProc.exe"
  nsExec::Exec "$INSTDIR\KillProc.exe TogglDesktop"
  Delete "$INSTDIR\KillProc.exe"
  StrCmp $0 "-1" wooops

  Goto completed

  wooops:
  DetailPrint "-> Error: Something went wrong :-("
  Abort

  completed:
  DetailPrint "Everything went okay :-D"

  ;Rename Bugsnag so we can update
  Rename $INSTDIR\Bugsnag.dll $INSTDIR\Bugsnag.1.2.dll

  ;Delete the old Bugsnag file on reboot
  Delete /REBOOTOK $INSTDIR\Bugsnag.1.2.dll

  ;ADD YOUR OWN FILES HERE...
  File "${redist}\*.dll"
  File "${srcdir}\*.dll"
  File "${srcdir}\*.exe"
  File "${srcdir}\cacert.pem"
  File "${srcdir}\TogglDesktop.exe.config"
  File "..\..\src\ui\windows\TogglDesktop\TogglDesktop\Resources\toggl.ico"

  ;Store installation folder
  WriteRegStr HKCU "Software\TogglDesktop" "" $INSTDIR

  ;Create uninstaller
!ifndef INNER
  SetOutPath $INSTDIR
  ; this packages the signed uninstaller
  File $%TEMP%\Uninstall.exe
!endif

  ;Create Desktop shortcut only when shortcut is present or at first install
  IfFileExists $DESKTOP\TogglDesktop.lnk 0 ShortcutDoesntExist
    CreateShortCut "$DESKTOP\TogglDesktop.lnk" "$INSTDIR\TogglDesktop.exe" ""
    ShortcutDoesntExist:
    ${If} $isUpdater == 0
      ;Create desktop shortcut
      CreateShortCut "$DESKTOP\TogglDesktop.lnk" "$INSTDIR\TogglDesktop.exe" ""
    ${EndIf}

  ;Add/Remove programs entry
  !define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\TogglDesktop"
  WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayName" "Toggl Desktop"
  WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayIcon" "$\"$INSTDIR\TogglDesktop.exe$\""
  WriteRegStr HKCU "${REG_UNINSTALL}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
  WriteRegStr HKCU "${REG_UNINSTALL}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKCU "${REG_UNINSTALL}" "Publisher" "Toggl"
  WriteRegStr HKCU "${REG_UNINSTALL}" "HelpLink" "https://support.toggl.com/desktop-apps"
  WriteRegStr HKCU "${REG_UNINSTALL}" "URLInfoAbout" "https://www.toggl.com/"
  WriteRegStr HKCU "${REG_UNINSTALL}" "InstallLocation" "$\"$INSTDIR$\""
  WriteRegStr HKCU "${REG_UNINSTALL}" "NoModify" 1
  WriteRegStr HKCU "${REG_UNINSTALL}" "NoRepair" 1
  WriteRegStr HKCU "${REG_UNINSTALL}" "Comments" "Uninstalls Toggl Desktop"

  ;Create start menu entry
  createDirectory "$SMPROGRAMS\Toggl"
  createShortCut "$SMPROGRAMS\Toggl\Toggl Desktop.lnk" "$INSTDIR\TogglDesktop.exe" "" "$INSTDIR\toggl.ico"
  createShortCut "$SMPROGRAMS\Toggl\Uninstall Toggl Desktop.lnk" "$INSTDIR\uninstall.exe" "" ""

SectionEnd

;--------------------------------
;Descriptions


;--------------------------------
;Uninstaller Section
!ifdef INNER
Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.xml"
  Delete "$INSTDIR\cacert.pem"
  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\TogglDesktop.exe.config"
  Delete "$INSTDIR\toggl.ico"

  ;Delete desktop shortcut
  Delete "$DESKTOP\TogglDesktop.lnk"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\TogglDesktop"

  ;Remove uninstall info from Control Panel
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\TogglDesktop"
  
  ; Remove run on Windows login entry
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "TogglDesktop"

  ;remove start menu links
  Delete "$SMPROGRAMS\Toggl\Toggl Desktop.lnk"
  Delete "$SMPROGRAMS\Toggl\Uninstall Toggl Desktop.lnk"
  RMDir "$SMPROGRAMS\Toggl"

SectionEnd
!endif

Function .onInit

!ifdef INNER
  ; If INNER is defined, then we aren't supposed to do anything except write out
  ; the uninstaller.  This is better than processing a command line option as it means
  ; this entire code path is not present in the final (real) installer.
  SetSilent silent
  WriteUninstaller "$%TEMP%\Uninstall.exe"
  Quit  ; just bail out quickly when running the "inner" installer
!endif

  ${GetParameters} $cmdLineParams
  Call checkOldVersion
  Call checkUpdater

FunctionEnd

Function checkOldVersion

  StrCpy $fromOldVersion 0

  ReadRegStr $3 HKLM "SOFTWARE\Toggl\TogglDesktop" "Version"
  StrLen $keyLength $3

  ${if} $keyLength != 0
    StrCpy $R3 $3 3
    StrCmp $R3 "7.1" 0 Newer
    StrCpy $fromOldVersion 1
    Newer:
  ${Endif}

FunctionEnd

Function checkUpdater

  Push $R0
  StrCpy $isUpdater 0
  ${GetOptions} $cmdLineParams '/U' $R0
  IfErrors +3 0
  StrCpy $isUpdater 1
  SetSilent silent

FunctionEnd

Function .onInstSuccess

  ${If} $fromOldVersion == 1
    ;Copy local database from 7.1 app to newer app location
    CopyFiles "$PROFILE\AppData\Roaming\Kopsik\kopsik.db" "$INSTDIR\toggldesktop.db"
  ${EndIf}
  
  ${if} $isUpdater == 1
    Exec "$INSTDIR\TogglDesktop.exe --updated"
  ${Endif}

FunctionEnd

Function runOnStartup
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "TogglDesktop" '"$INSTDIR\TogglDesktop.exe" --minimize'
FunctionEnd

; Don't allow user to change install dir as we only allow user dir for install
Function DisableInstallPathEdit

  FindWindow $R0 "#32770" "" $HWNDPARENT
  GetDlgItem $R1 $R0 1019
    SendMessage $R1 ${EM_SETREADONLY} 1 0
  GetDlgItem $R1 $R0 1001
    EnableWindow $R1 0

FunctionEnd

Function un.customPage
  !insertmacro MUI_HEADER_TEXT "Uninstall Toggl Desktop" "Remove Toggl Desktop from your computer."
  StrCpy $deleteData 0
  nsDialogs::Create 1018
  Pop $0

  ${NSD_CreateLabel} 0 0 100% 40u "Toggl Desktop will be uninstalled from the following folder. Click Uninstall to start the uninstallation."

  ${NSD_CreateLabel} 0 30% 20% 12u "Uninstalling from:"

  ${NSD_CreateText} 20% 28% 80% 12u "$INSTDIR"
  Pop $7
  EnableWindow $7 0 # text field is disabled

  ${NSD_CreateCheckbox} 0 -50 100% 8u "Remove also all local data"
  Pop $CHECKBOX
  GetFunctionAddress $0 un.OnCheckbox
  nsDialogs::OnClick $CHECKBOX $0

  nsDialogs::Show

FunctionEnd

Function un.OnCheckbox

  Pop $0 # HWND

  ${If} $deleteData == 1
    StrCpy $deleteData 0
  ${Else}
    StrCpy $deleteData 1
  ${EndIf}

FunctionEnd

Function un.onUninstSuccess

  ${If} $deleteData == 1
    Delete "$INSTDIR\toggldesktop.db"
    Delete "$INSTDIR\toggldesktop.log"
    RMDir /r $INSTDIR
  ${EndIf} 

FunctionEnd
