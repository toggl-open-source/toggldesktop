
;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"

;Include FileFunc for GetParameters
  !include "FileFunc.nsh"

;Include LogicLib for if statements
  !include 'LogicLib.nsh'

;Include nsDialogs for custom Uninstaller page
  !include nsDialogs.nsh
  
;Add a plugin directory
  ;!addplugindir NSIS_plugins

;--------------------------------
;Add Macros

  !insertmacro GetParameters
  !insertmacro GetOptions

;--------------------------------
;Global variables

  Var isOldUpdater
  Var isNewUpdater
  Var deleteData
  Var CHECKBOX
  Var cmdLineParams

;--------------------------------
;General

  Name "Toggl Track"
  OutFile "TogglDesktopInstaller-x64.exe"
  SetCompressor /SOLID lzma

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
  !define MUI_FINISHPAGE_RUN_TEXT "Launch Toggl Track"

;Install Location page
  !define MUI_PAGE_HEADER_TEXT "Install location"
  !define MUI_PAGE_HEADER_SUBTEXT ""
  !define MUI_DIRECTORYPAGE_TEXT_TOP "Setup will install Toggl Track into $INSTDIR. Click Install to start the installation"

;Run App on Windows login
  !define MUI_FINISHPAGE_SHOWREADME
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "Run Toggl Track on Windows login"
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

!define srcdir "..\..\src\ui\windows\TogglDesktop\TogglDesktop\bin\x64\Release"
!define redist "..\..\third_party\vs_redist\x64"

Section

  SetOutPath "$INSTDIR"

  ${If} $isNewUpdater == 0
	;Check if Old version of the app is still running and close it
	;KillProcDLL::KillProc "TogglDesktop.exe"
    
    ; Delete the main executable to prevent it from being launched while an update is running
    Delete "$INSTDIR\TogglDesktop.exe"
  ${EndIf}
  
  ;ADD YOUR OWN FILES HERE...
  File "${redist}\*.dll"
  File "${srcdir}\*.dll"
  File "${srcdir}\*.exe"
  File "${srcdir}\cacert.pem"
  File "${srcdir}\TogglDesktop.exe.config"
  File "..\..\src\ui\windows\TogglDesktop\TogglDesktop\Resources\toggl.ico"

  ${If} $isOldUpdater == 0
  ${AndIf} $isNewUpdater == 0    
    ;Store installation folder
    WriteRegStr HKCU "Software\TogglDesktop" "" $INSTDIR
  ${EndIf}

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Create Desktop shortcut at first install
  ${If} $isOldUpdater == 0
  ${AndIf} $isNewUpdater == 0
    CreateShortCut "$DESKTOP\Toggl Track.lnk" "$INSTDIR\TogglDesktop.exe" ""
  ${Else}
    IfFileExists "$DESKTOP\TogglDesktop.lnk" 0 +2
    Rename /REBOOTOK "$DESKTOP\TogglDesktop.lnk" "$DESKTOP\Toggl Track.lnk"
  ${EndIf}

  ${If} $isOldUpdater == 0
  ${AndIf} $isNewUpdater == 0
    ;Add/Remove programs entry
    !define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\TogglDesktop"
    WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayName" "Toggl Track"
    WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayIcon" "$\"$INSTDIR\TogglDesktop.exe$\""
    WriteRegStr HKCU "${REG_UNINSTALL}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegStr HKCU "${REG_UNINSTALL}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKCU "${REG_UNINSTALL}" "Publisher" "Toggl"
    WriteRegStr HKCU "${REG_UNINSTALL}" "HelpLink" "https://support.toggl.com/desktop-apps"
    WriteRegStr HKCU "${REG_UNINSTALL}" "URLInfoAbout" "https://www.toggl.com/"
    WriteRegStr HKCU "${REG_UNINSTALL}" "InstallLocation" "$\"$INSTDIR$\""
    WriteRegStr HKCU "${REG_UNINSTALL}" "NoModify" 1
    WriteRegStr HKCU "${REG_UNINSTALL}" "NoRepair" 1
    WriteRegStr HKCU "${REG_UNINSTALL}" "Comments" "Uninstalls Toggl Track"

    ;Create start menu entry
    createDirectory "$SMPROGRAMS\Toggl"
    createShortCut "$SMPROGRAMS\Toggl\Toggl Track.lnk" "$INSTDIR\TogglDesktop.exe" "" "$INSTDIR\toggl.ico"
    createShortCut "$SMPROGRAMS\Toggl\Uninstall Toggl Track.lnk" "$INSTDIR\uninstall.exe" "" ""
  ${EndIf}
SectionEnd

;--------------------------------
;Descriptions


;--------------------------------
;Uninstaller Section
Section "Uninstall"

  Call un.killAppProcess
  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.xml"
  Delete "$INSTDIR\cacert.pem"
  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\TogglDesktop.exe.config"
  Delete "$INSTDIR\toggl.ico"
  RMDir "$INSTDIR\updates"
  RMDir /r "$LOCALAPPDATA\Onova\TogglDesktop" ;Remove the prepared updates

  ;Delete desktop shortcut
  Delete "$DESKTOP\Toggl Track.lnk"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\TogglDesktop"

  ;Remove uninstall info from Control Panel
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\TogglDesktop"

  ; Remove run on Windows login entry
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "TogglDesktop"

  ;remove start menu links
  Delete "$SMPROGRAMS\Toggl\Toggl Desktop.lnk"
  Delete "$SMPROGRAMS\Toggl\Toggl Track.lnk"
  Delete "$SMPROGRAMS\Toggl\Uninstall Toggl Desktop.lnk"
  Delete "$SMPROGRAMS\Toggl\Uninstall Toggl Track.lnk"
  RMDir "$SMPROGRAMS\Toggl"

SectionEnd

Function .onInit

  ${GetParameters} $cmdLineParams
  Call checkUpdater

FunctionEnd

Function checkUpdater

  Push $R0
  StrCpy $isOldUpdater 0
  ${GetOptions} $cmdLineParams '/U' $R0
  IfErrors +3 0
  StrCpy $isOldUpdater 1
  SetSilent silent
  StrCpy $isNewUpdater 0
  ${GetOptions} $cmdLineParams "/autoupdate" $R0
  IfErrors +3 0
  StrCpy $isNewUpdater 1
  SetSilent silent

FunctionEnd

Function .onInstSuccess

  ${if} $isOldUpdater == 1
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

Function un.killAppProcess
  ;Check if Old version of the app is still running and close it
	;KillProcDLL::KillProc "TogglDesktop.exe"
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
