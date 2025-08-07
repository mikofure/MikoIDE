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
  Var AddToPath

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
  File "build\Release\MikoIDE.exe"
  
  ;Create bin directory
  CreateDirectory "$INSTDIR\bin"
  SetOutPath "$INSTDIR\bin"
  File "build\Release\bin\miko.exe"
  File "build\Release\bin\miko.cmd"

  SetOutPath "$INSTDIR"

  ;Core DLL files
  File "build\Release\chrome_elf.dll"
  File "build\Release\d3dcompiler_47.dll"
  File "build\Release\libcef.dll"
  File "build\Release\libEGL.dll"
  File "build\Release\libGLESv2.dll"
  File "build\Release\shared.lib"
  File "build\Release\shared.pdb"
  File "build\Release\vk_swiftshader.dll"
  File "build\Release\vulkan-1.dll"

  ;Data files
  File "build\Release\chrome_100_percent.pak"
  File "build\Release\chrome_200_percent.pak"
  File "build\Release\icudtl.dat"
  File "build\Release\resources.pak"
  File "build\Release\snapshot_blob.bin"
  File "build\Release\v8_context_snapshot.bin"
  File "build\Release\vk_swiftshader_icd.json"

  ;Assets folder
  SetOutPath "$INSTDIR\assets"
  File /r "build\Release\assets\*.*"

  ;Locales folder
  SetOutPath "$INSTDIR\locales"
  File /r /x "DawnCache" /x "GPUCache" "build\Release\locales\*.*"

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

Section "Add to PATH Environment Variable" SecPath

  ; Add MikoIDE bin directory to user PATH
  ReadRegStr $0 HKCU "Environment" "PATH"
  StrCmp $0 "" AddToPath_NoPrevPath
    StrCpy $0 "$0;$INSTDIR\bin"
    Goto AddToPath_WriteReg
  AddToPath_NoPrevPath:
    StrCpy $0 "$INSTDIR\bin"
  AddToPath_WriteReg:
    WriteRegExpandStr HKCU "Environment" "PATH" $0
    
  ; Notify system of environment variable change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  
  ; Store that PATH was modified for uninstaller
  WriteRegStr HKCU "Software\MikoIDE" "PathAdded" "1"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecCore ${LANG_ENGLISH} "Core MikoIDE application files (required)."
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Create a desktop shortcut for MikoIDE."
  LangString DESC_SecStartMenu ${LANG_ENGLISH} "Create Start Menu shortcuts for MikoIDE."
  LangString DESC_SecPath ${LANG_ENGLISH} "Add MikoIDE bin directory to PATH environment variable for command line access."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPath} $(DESC_SecPath)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;Remove files and uninstaller
  Delete "$INSTDIR\MikoIDE.exe"
  Delete "$INSTDIR\bin\miko.exe"
  Delete "$INSTDIR\bin\miko.cmd"
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
  RMDir "$INSTDIR\bin"
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

  ;Remove from PATH if it was added
  ReadRegStr $0 HKCU "Software\MikoIDE" "PathAdded"
  StrCmp $0 "1" 0 SkipPathRemoval
    
    ; Remove from PATH using simple string replacement
    ReadRegStr $0 HKCU "Environment" "PATH"
    
    ; Try to remove ";$INSTDIR\bin" first
    StrCpy $1 $0
    StrCpy $2 ";$INSTDIR\bin"
    Call un.RemoveFromPath
    StrCpy $0 $3
    
    ; Try to remove "$INSTDIR\bin;" 
    StrCpy $1 $0
    StrCpy $2 "$INSTDIR\bin;"
    Call un.RemoveFromPath
    StrCpy $0 $3
    
    ; Try to remove "$INSTDIR\bin" (standalone)
    StrCpy $1 $0
    StrCpy $2 "$INSTDIR\bin"
    Call un.RemoveFromPath
    StrCpy $0 $3
    
    WriteRegExpandStr HKCU "Environment" "PATH" $0
    
    ; Notify system of environment variable change
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    
  SkipPathRemoval:

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

;--------------------------------
;String replacement function for uninstaller

Function un.RemoveFromPath
  ; Input: $1 = original string, $2 = substring to remove
  ; Output: $3 = result string
  
  StrCpy $3 $1
  StrLen $4 $2
  StrLen $5 $1
  
  ; If substring is longer than original string, nothing to remove
  IntCmp $4 $5 CheckEqual CheckDone CheckDone
  
  CheckEqual:
    StrCmp $1 $2 RemoveAll CheckDone
  
  RemoveAll:
    StrCpy $3 ""
    Goto CheckDone
    
  ; Find and replace substring
  StrCpy $6 0  ; position counter
  
  FindLoop:
    IntCmp $6 $5 CheckDone CheckDone 0
    StrCpy $7 $1 $4 $6  ; extract substring of length $4 at position $6
    StrCmp $7 $2 Found NotFound
    
  Found:
    ; Build result: part before + part after
    StrCpy $8 $1 $6  ; part before
    IntOp $9 $6 + $4  ; position after substring
    StrCpy $R0 $1 "" $9  ; part after
    StrCpy $3 "$8$R0"
    Goto CheckDone
    
  NotFound:
    IntOp $6 $6 + 1
    Goto FindLoop
    
  CheckDone:
FunctionEnd