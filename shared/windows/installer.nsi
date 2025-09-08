; MikoIDE NSIS Installer Script
; Modern UI 2.0

!include "MUI2.nsh"

; General
Name "MikoIDE"
OutFile "MikoIDE-Setup.exe"
Unicode True

; Default installation folder
InstallDir "$LOCALAPPDATA\MikoIDE"

; Get installation folder from registry if available
InstallDirRegKey HKCU "Software\MikoIDE" ""

; Request application privileges
RequestExecutionLevel user

; Variables
Var StartMenuFolder

; Interface Settings
!define MUI_ABORTWARNING
!define MUI_ICON "assets\icon.ico"
!define MUI_UNICON "assets\icon.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"

; Components page
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\MikoIDE"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

; Installation page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\MikoIDE.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch MikoIDE"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

; Version Information
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "MikoIDE"
VIAddVersionKey "CompanyName" "Mikofure Project"
VIAddVersionKey "LegalCopyright" "Copyright 2025 Mikofure Project"
VIAddVersionKey "FileDescription" "MikoIDE Installer"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "ProductVersion" "1.0.0.0"

; Installer sections
Section "MikoIDE Core" SecCore
  SectionIn RO
  
  ; Set output path to the installation directory
  SetOutPath "$INSTDIR"
  
  ; Install main application files
  File /r "..\..\build\Release\*"
  
  ; Store installation folder
  WriteRegStr HKCU "Software\MikoIDE" "" $INSTDIR
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Add to Add/Remove Programs
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "DisplayName" "MikoIDE"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "DisplayIcon" "$INSTDIR\MikoIDE.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "Publisher" "Mikofure Project"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "DisplayVersion" "1.0.0"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE" "NoRepair" 1
  
SectionEnd

Section "Desktop Shortcut" SecDesktop
  CreateShortcut "$DESKTOP\MikoIDE.lnk" "$INSTDIR\MikoIDE.exe" "" "$INSTDIR\MikoIDE.exe" 0
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ; Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\MikoIDE.lnk" "$INSTDIR\MikoIDE.exe" "" "$INSTDIR\MikoIDE.exe" 0
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall MikoIDE.lnk" "$INSTDIR\Uninstall.exe"
    
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "File Associations" SecFileAssoc
  ; Register file associations for common programming languages
  WriteRegStr HKCU "Software\Classes\.c" "" "MikoIDE.SourceFile"
  WriteRegStr HKCU "Software\Classes\.cpp" "" "MikoIDE.SourceFile"
  WriteRegStr HKCU "Software\Classes\.cxx" "" "MikoIDE.SourceFile"
  WriteRegStr HKCU "Software\Classes\.cc" "" "MikoIDE.SourceFile"
  WriteRegStr HKCU "Software\Classes\.h" "" "MikoIDE.HeaderFile"
  WriteRegStr HKCU "Software\Classes\.hpp" "" "MikoIDE.HeaderFile"
  WriteRegStr HKCU "Software\Classes\.hxx" "" "MikoIDE.HeaderFile"
  WriteRegStr HKCU "Software\Classes\.ts" "" "MikoIDE.TypeScriptFile"
  WriteRegStr HKCU "Software\Classes\.tsx" "" "MikoIDE.ReactFile"
  WriteRegStr HKCU "Software\Classes\.jsx" "" "MikoIDE.ReactFile"
  WriteRegStr HKCU "Software\Classes\.js" "" "MikoIDE.JavaScriptFile"
  WriteRegStr HKCU "Software\Classes\.json" "" "MikoIDE.JSONFile"
  
  ; Define file type descriptions
  WriteRegStr HKCU "Software\Classes\MikoIDE.SourceFile" "" "C/C++ Source File"
  WriteRegStr HKCU "Software\Classes\MikoIDE.SourceFile\DefaultIcon" "" "$INSTDIR\fileicon\icon_c.ico,0"
  WriteRegStr HKCU "Software\Classes\MikoIDE.SourceFile\shell\open\command" "" '"$INSTDIR\MikoIDE.exe" "%1"'
  
  WriteRegStr HKCU "Software\Classes\MikoIDE.HeaderFile" "" "C/C++ Header File"
  WriteRegStr HKCU "Software\Classes\MikoIDE.HeaderFile\DefaultIcon" "" "$INSTDIR\fileicon\icon_cpp.ico,0"
  WriteRegStr HKCU "Software\Classes\MikoIDE.HeaderFile\shell\open\command" "" '"$INSTDIR\MikoIDE.exe" "%1"'
  
  WriteRegStr HKCU "Software\Classes\MikoIDE.TypeScriptFile" "" "TypeScript File"
  WriteRegStr HKCU "Software\Classes\MikoIDE.TypeScriptFile\DefaultIcon" "" "$INSTDIR\fileicon\icon_typescript.ico,0"
  WriteRegStr HKCU "Software\Classes\MikoIDE.TypeScriptFile\shell\open\command" "" '"$INSTDIR\MikoIDE.exe" "%1"'
  
  WriteRegStr HKCU "Software\Classes\MikoIDE.JavaScriptFile" "" "JavaScript File"
  WriteRegStr HKCU "Software\Classes\MikoIDE.JavaScriptFile\DefaultIcon" "" "$INSTDIR\fileicon\icon_javascript.ico,0"
  WriteRegStr HKCU "Software\Classes\MikoIDE.JavaScriptFile\shell\open\command" "" '"$INSTDIR\MikoIDE.exe" "%1"'
  
  WriteRegStr HKCU "Software\Classes\MikoIDE.ReactFile" "" "React File"
  WriteRegStr HKCU "Software\Classes\MikoIDE.ReactFile\DefaultIcon" "" "$INSTDIR\fileicon\icon_react.ico,0"
  WriteRegStr HKCU "Software\Classes\MikoIDE.ReactFile\shell\open\command" "" '"$INSTDIR\MikoIDE.exe" "%1"'
  
  WriteRegStr HKCU "Software\Classes\MikoIDE.JSONFile" "" "JSON File"
  WriteRegStr HKCU "Software\Classes\MikoIDE.JSONFile\DefaultIcon" "" "$INSTDIR\MikoIDE.exe,0"
  WriteRegStr HKCU "Software\Classes\MikoIDE.JSONFile\shell\open\command" "" '"$INSTDIR\MikoIDE.exe" "%1"'
  
  ; Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core MikoIDE application files (required)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create a desktop shortcut for MikoIDE"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Create Start Menu shortcuts for MikoIDE"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "Associate common programming file types with MikoIDE"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller section
Section "Uninstall"
  
  ; Remove files and uninstaller
  RMDir /r "$INSTDIR"
  
  ; Remove shortcuts
  Delete "$DESKTOP\MikoIDE.lnk"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  
  Delete "$SMPROGRAMS\$StartMenuFolder\MikoIDE.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall MikoIDE.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MikoIDE"
  DeleteRegKey HKCU "Software\MikoIDE"
  
  ; Remove file associations
  DeleteRegKey HKCU "Software\Classes\.c"
  DeleteRegKey HKCU "Software\Classes\.cpp"
  DeleteRegKey HKCU "Software\Classes\.cxx"
  DeleteRegKey HKCU "Software\Classes\.cc"
  DeleteRegKey HKCU "Software\Classes\.h"
  DeleteRegKey HKCU "Software\Classes\.hpp"
  DeleteRegKey HKCU "Software\Classes\.hxx"
  DeleteRegKey HKCU "Software\Classes\.ts"
  DeleteRegKey HKCU "Software\Classes\.tsx"
  DeleteRegKey HKCU "Software\Classes\.jsx"
  DeleteRegKey HKCU "Software\Classes\.js"
  DeleteRegKey HKCU "Software\Classes\.json"
  DeleteRegKey HKCU "Software\Classes\MikoIDE.SourceFile"
  DeleteRegKey HKCU "Software\Classes\MikoIDE.HeaderFile"
  DeleteRegKey HKCU "Software\Classes\MikoIDE.TypeScriptFile"
  DeleteRegKey HKCU "Software\Classes\MikoIDE.ReactFile"
  DeleteRegKey HKCU "Software\Classes\MikoIDE.JavaScriptFile"
  DeleteRegKey HKCU "Software\Classes\MikoIDE.JSONFile"
  
  ; Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
  
SectionEnd