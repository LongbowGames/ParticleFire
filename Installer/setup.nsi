!addPluginDir "."

; Include modern UI
!include "MUI.nsh"
!include "WinVer.nsh"
!include "x64.nsh"

!define BANNER "Banner.bmp"

; ParallaxTZ's RMDirUp starts
Function un.RMDirUP
         !define RMDirUP '!insertmacro RMDirUPCall'
 
         !macro RMDirUPCall _PATH
                push '${_PATH}'
                Call un.RMDirUP
         !macroend
 
         ; $0 - current folder
         ClearErrors
 
         Exch $0
         ;DetailPrint "ASDF - $0\.."
         RMDir "$0\.."
         
         IfErrors Skip
         ${RMDirUP} "$0\.."
         Skip:
         
         Pop $0
FunctionEnd
; ParallaxTZ's RMDirUp ends

; REGKEY is used under both HKLM and NKCU (the latter for the IGF install only)
!define PRODUCT "Particle Fire 2"
!define REGKEY "Software\Longbow Digital Arts\${PRODUCT}"
!define UNINSTKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
!define WEBSITE "http://www.longbowgames.com/"

; Name of the installer, output file, and default directory/registry key
Name "${PRODUCT}"
OutFile "Out/${PRODUCT} Install.exe"
InstallDir "$PROGRAMFILES\Longbow Digital Arts\${PRODUCT}"
InstallDirRegKey HKLM "${REGKEY}" "Install_Dir"
SetCompressor /SOLID lzma
RequestExecutionLevel admin

; misc
!define MUI_ICON "install.ico"
!define MUI_UNICON "uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_UNHEADERIMAGE_BITMAP "header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP ${BANNER}
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ${BANNER}
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_ABORTWARNING

; pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; instfiles page *****************
; The stuff to install
Section "${PRODUCT} (Required)"
  SetShellVarContext all
  SectionIn RO

  ; All the files we want
  SetOutPath "$INSTDIR"
  File ..\README
  File ..\LICENSE
  ${DisableX64FSRedirection}
  SetOutPath "$SYSDIR"
  File "..\Release\Particle Fire.scr"

  ; Write the installation path into the registry
  WriteRegStr HKLM "${REGKEY}" "Install_Dir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${UNINSTKEY}" "DisplayName" "${PRODUCT}"
  WriteRegStr HKLM "${UNINSTKEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd

Section "Set Particle Fire as your new screen saver" set_default
  SetShellVarContext current
  ${DisableX64FSRedirection}
  WriteRegStr HKCU "Control Panel\Desktop" "Scrnsave.exe" "$SYSDIR\Particle Fire 2.scr"
  WriteRegStr HKCU "Control Panel\Desktop" "ScreenSaveActive" "1"
SectionEnd

; uninstall stuff

; special uninstall section.
Section "Uninstall"
  SetShellVarContext all

  ; remove registry keys
  DeleteRegKey HKLM "${UNINSTKEY}"
  DeleteRegKey HKLM "${REGKEY}"

  ; remove files
  RMDir /r "$INSTDIR"
  ${RMDirUp} "$INSTDIR"
  ${DisableX64FSRedirection}
  Delete "$SYSDIR\Particle Fire 2.scr"
SectionEnd

Function .onInit
  ; Make sure they're an administrator
  ClearErrors
  UserInfo::GetName
  ${Unless} ${Errors}
    Pop $0
    UserInfo::GetAccountType
    Pop $1

    ${If} $1 != "Admin"
      MessageBox MB_OK|MB_ICONEXCLAMATION "You must be an administrator to install ${PRODUCT}."
      Abort
    ${EndIf}
  ${EndUnless}
FunctionEnd

