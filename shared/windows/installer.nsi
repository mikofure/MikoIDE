; Hyperion Installer Script
; This script creates an installer for Hyperion targeting LocalAppData

!include "MUI2.nsh"

; General
Name "Hyperion"
OutFile "Hyperion-Setup.exe"
Unicode True

; Default installation folder
InstallDir "$LOCALAPPDATA\Hyperion"

; Registry key to check for directory (if you have used Inno Setup before)
InstallDirRegKey HKCU "Software\Hyperion" ""

; Request application privileges for Windows Vista and higher
RequestExecutionLevel user

; Variables
Var StartMenuFolder

; Interface Settings
!define MUI_ABORTWARNING

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

; Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Hyperion" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

; Installer sections
Section "Hyperion Core" SecCore
  SectionIn RO
  
  ; Set output path to the installation directory
  SetOutPath "$INSTDIR"
  
  ; Copy main executables
  File "..\..\build\Release\Hyperion.exe"
  File "..\..\build\Release\mikoterminal.exe"
  File "..\..\build\Release\SDL3.dll"
  
  ; Create bin directory and copy files
  SetOutPath "$INSTDIR\bin"
  File "..\..\build\Release\bin\hyprn.exe"
  
  ; Create bin\cef\windows64 directory (if CEF files exist)
  CreateDirectory "$INSTDIR\bin\cef\windows64"
  ; Note: CEF files would be copied here if they exist
  ; File /r "..\..\build\Release\bin\cef\windows64\*.*"
  
  ; Create tools directory
  SetOutPath "$INSTDIR\tools"
  File "..\..\build\Release\tools\termibench.exe"
  
  ; Copy shared resources
  SetOutPath "$INSTDIR\shared\icon"
  File "..\icon\*.*"
  
  SetOutPath "$INSTDIR\shared\theme"
  File /r "..\theme\*.*"
  
  ; Store installation folder
  WriteRegStr HKCU "Software\Hyperion" "" $INSTDIR
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Add to Add/Remove Programs
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion" "DisplayName" "Hyperion"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion" "DisplayIcon" "$INSTDIR\Hyperion.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion" "Publisher" "Hyperion Team"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion" "NoRepair" 1

SectionEnd

Section "Desktop Shortcut" SecDesktop
  CreateShortcut "$DESKTOP\Hyperion.lnk" "$INSTDIR\Hyperion.exe"
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ; Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Hyperion.lnk" "$INSTDIR\Hyperion.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\MikoTerminal.lnk" "$INSTDIR\mikoterminal.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "File Associations" SecFileAssoc
  ; Function to register file association
  !macro RegisterExtension EXT DESCRIPTION
    WriteRegStr HKCU "Software\Classes\.${EXT}" "" "Hyperion.${EXT}"
    WriteRegStr HKCU "Software\Classes\Hyperion.${EXT}" "" "${DESCRIPTION}"
    WriteRegStr HKCU "Software\Classes\Hyperion.${EXT}\DefaultIcon" "" "$INSTDIR\Hyperion.exe,0"
    WriteRegStr HKCU "Software\Classes\Hyperion.${EXT}\shell\open\command" "" '"$INSTDIR\Hyperion.exe" "%1"'
  !macroend
  
  ; Associate common programming file extensions with Hyperion
  !insertmacro RegisterExtension "c" "C Source File"
  !insertmacro RegisterExtension "cpp" "C++ Source File"
  !insertmacro RegisterExtension "cxx" "C++ Source File"
  !insertmacro RegisterExtension "cc" "C++ Source File"
  !insertmacro RegisterExtension "h" "C Header File"
  !insertmacro RegisterExtension "hpp" "C++ Header File"
  !insertmacro RegisterExtension "hxx" "C++ Header File"
  !insertmacro RegisterExtension "js" "JavaScript File"
  !insertmacro RegisterExtension "ts" "TypeScript File"
  !insertmacro RegisterExtension "jsx" "React JavaScript File"
  !insertmacro RegisterExtension "tsx" "React TypeScript File"
  !insertmacro RegisterExtension "py" "Python File"
  !insertmacro RegisterExtension "go" "Go Source File"
  !insertmacro RegisterExtension "rs" "Rust Source File"
  !insertmacro RegisterExtension "java" "Java Source File"
  !insertmacro RegisterExtension "cs" "C# Source File"
  !insertmacro RegisterExtension "php" "PHP File"
  !insertmacro RegisterExtension "rb" "Ruby File"
  !insertmacro RegisterExtension "vue" "Vue Component"
  !insertmacro RegisterExtension "json" "JSON File"
  !insertmacro RegisterExtension "xml" "XML File"
  !insertmacro RegisterExtension "html" "HTML File"
  !insertmacro RegisterExtension "htm" "HTML File"
  !insertmacro RegisterExtension "css" "CSS File"
  !insertmacro RegisterExtension "scss" "SCSS File"
  !insertmacro RegisterExtension "sass" "SASS File"
  !insertmacro RegisterExtension "less" "LESS File"
  !insertmacro RegisterExtension "md" "Markdown File"
  !insertmacro RegisterExtension "txt" "Text File"
  !insertmacro RegisterExtension "log" "Log File"
  !insertmacro RegisterExtension "ini" "Configuration File"
  !insertmacro RegisterExtension "cfg" "Configuration File"
  !insertmacro RegisterExtension "conf" "Configuration File"
  !insertmacro RegisterExtension "yaml" "YAML File"
  !insertmacro RegisterExtension "yml" "YAML File"
  !insertmacro RegisterExtension "toml" "TOML File"
  
  ; Refresh shell to update file associations
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
SectionEnd

; Descriptions
LangString DESC_SecCore ${LANG_ENGLISH} "Core Hyperion application files (required)"
LangString DESC_SecDesktop ${LANG_ENGLISH} "Create a desktop shortcut for Hyperion"
LangString DESC_SecStartMenu ${LANG_ENGLISH} "Create Start Menu shortcuts"
LangString DESC_SecFileAssoc ${LANG_ENGLISH} "Associate common programming file types with Hyperion"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} $(DESC_SecFileAssoc)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller section
Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hyperion"
  DeleteRegKey HKCU "Software\Hyperion"

  ; Remove file associations
  !macro UnregisterExtension EXT
    DeleteRegKey HKCU "Software\Classes\.${EXT}"
    DeleteRegKey HKCU "Software\Classes\Hyperion.${EXT}"
  !macroend
  
  !insertmacro UnregisterExtension "c"
  !insertmacro UnregisterExtension "cpp"
  !insertmacro UnregisterExtension "cxx"
  !insertmacro UnregisterExtension "cc"
  !insertmacro UnregisterExtension "h"
  !insertmacro UnregisterExtension "hpp"
  !insertmacro UnregisterExtension "hxx"
  !insertmacro UnregisterExtension "js"
  !insertmacro UnregisterExtension "ts"
  !insertmacro UnregisterExtension "jsx"
  !insertmacro UnregisterExtension "tsx"
  !insertmacro UnregisterExtension "py"
  !insertmacro UnregisterExtension "go"
  !insertmacro UnregisterExtension "rs"
  !insertmacro UnregisterExtension "java"
  !insertmacro UnregisterExtension "cs"
  !insertmacro UnregisterExtension "php"
  !insertmacro UnregisterExtension "rb"
  !insertmacro UnregisterExtension "vue"
  !insertmacro UnregisterExtension "json"
  !insertmacro UnregisterExtension "xml"
  !insertmacro UnregisterExtension "html"
  !insertmacro UnregisterExtension "htm"
  !insertmacro UnregisterExtension "css"
  !insertmacro UnregisterExtension "scss"
  !insertmacro UnregisterExtension "sass"
  !insertmacro UnregisterExtension "less"
  !insertmacro UnregisterExtension "md"
  !insertmacro UnregisterExtension "txt"
  !insertmacro UnregisterExtension "log"
  !insertmacro UnregisterExtension "ini"
  !insertmacro UnregisterExtension "cfg"
  !insertmacro UnregisterExtension "conf"
  !insertmacro UnregisterExtension "yaml"
  !insertmacro UnregisterExtension "yml"
  !insertmacro UnregisterExtension "toml"

  ; Remove files and uninstaller
  Delete "$INSTDIR\Hyperion.exe"
  Delete "$INSTDIR\mikoterminal.exe"
  Delete "$INSTDIR\SDL3.dll"
  Delete "$INSTDIR\bin\hyprn.exe"
  Delete "$INSTDIR\tools\termibench.exe"
  Delete "$INSTDIR\Uninstall.exe"
  
  ; Remove shared resources
  RMDir /r "$INSTDIR\shared"
  RMDir /r "$INSTDIR\bin\cef"
  RMDir "$INSTDIR\bin"
  RMDir "$INSTDIR\tools"

  ; Remove shortcuts
  Delete "$DESKTOP\Hyperion.lnk"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\Hyperion.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\MikoTerminal.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  ; Remove directories
  RMDir "$INSTDIR"

  ; Refresh shell to update file associations
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

; Function to run on installer initialization
Function .onInit
  ; Check if Hyperion is already installed
  ReadRegStr $R0 HKCU "Software\Hyperion" ""
  StrCmp $R0 "" done
  
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
    "Hyperion is already installed. $\n$\nClick `OK` to remove the previous version or `Cancel` to cancel this upgrade." \
    IDOK uninst
  Abort
  
  uninst:
    ClearErrors
    ExecWait '$R0\Uninstall.exe _?=$R0'
    
    IfErrors no_remove_uninstaller done
      Delete "$R0\Uninstall.exe"
      RMDir "$R0"
    no_remove_uninstaller:
  
  done:
FunctionEnd
