;NSIS Modern User Interface
;MikoIDE Installer Script

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "MikoIDE"
  OutFile "MikoIDE-Setup.exe"
  Unicode True

  ;Default installation folder
  InstallDir "$LOCALAPPDATA\MikoIDE"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\MikoIDE" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

  ;Show all languages, despite user's codepage
  !define MUI_LANGDLL_ALLLANGUAGES

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\MikoIDE"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "MikoIDE Core" SecCore

  SectionIn RO

  SetOutPath "$INSTDIR"

  ;Main executable
  File "MikoIDE.exe"

  ;Core DLL files
  File "chrome_elf.dll"
  File "d3dcompiler_47.dll"
  File "libcef.dll"
  File "libEGL.dll"
  File "libGLESv2.dll"
  File "shared.lib"
  File "shared.pdb"
  File "vk_swiftshader.dll"
  File "vulkan-1.dll"

  ;Data files
  File "chrome_100_percent.pak"
  File "chrome_200_percent.pak"
  File "icudtl.dat"
  File "resources.pak"
  File "snapshot_blob.bin"
  File "v8_context_snapshot.bin"
  File "vk_swiftshader_icd.json"

  ;Assets folder
  SetOutPath "$INSTDIR\assets"
  File /r "assets\*.*"

  ;Locales folder
  SetOutPath "$INSTDIR\locales"
  File /r "locales\*.*"

  ;Store installation folder
  WriteRegStr HKCU "Software\MikoIDE" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Add uninstall information to Add/Remove Programs
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "DisplayName" "MikoIDE"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "DisplayIcon" "$INSTDIR\MikoIDE.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "Publisher" "MikoIDE Team"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "DisplayVersion" "1.0.0"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "NoRepair" 1

SectionEnd

Section "Desktop Shortcut" SecDesktop

  CreateShortcut "$DESKTOP\MikoIDE.lnk" "$INSTDIR\MikoIDE.exe"

SectionEnd

Section "Start Menu Shortcuts" SecStartMenu

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\MikoIDE.lnk" "$INSTDIR\MikoIDE.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall MikoIDE.lnk" "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecCore ${LANG_ENGLISH} "Core MikoIDE application files (required)."
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Create a desktop shortcut for MikoIDE."
  LangString DESC_SecStartMenu ${LANG_ENGLISH} "Create Start Menu shortcuts for MikoIDE."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;Remove files and uninstaller
  Delete "$INSTDIR\MikoIDE.exe"
  Delete "$INSTDIR\chrome_elf.dll"
  Delete "$INSTDIR\d3dcompiler_47.dll"  
  Delete "$INSTDIR\libcef.dll"
  Delete "$INSTDIR\libEGL.dll"
  Delete "$INSTDIR\libGLESv2.dll"
  Delete "$INSTDIR\shared.lib"
  Delete "$INSTDIR\shared.pdb"
  Delete "$INSTDIR\vk_swiftshader.dll"
  Delete "$INSTDIR\vulkan-1.dll"
  Delete "$INSTDIR\chrome_100_percent.pak"
  Delete "$INSTDIR\chrome_200_percent.pak"
  Delete "$INSTDIR\icudtl.dat"
  Delete "$INSTDIR\resources.pak"
  Delete "$INSTDIR\snapshot_blob.bin"
  Delete "$INSTDIR\v8_context_snapshot.bin"
  Delete "$INSTDIR\vk_swiftshader_icd.json"

  ;Remove directories
  RMDir /r "$INSTDIR\assets"
  RMDir /r "$INSTDIR\locales"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  ;Remove shortcuts
  Delete "$DESKTOP\MikoIDE.lnk"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\MikoIDE.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall MikoIDE.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  ;Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE"
  DeleteRegKey /ifempty HKCU "Software\MikoIDE"

SectionEnd

;--------------------------------
;Installer Functions

Function .onInit

  ;Language selection dialog
  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

FunctionEnd