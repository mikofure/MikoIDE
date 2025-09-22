; FileAssociation.nsh
; File association helper macros
; Written by Saivert
; 
; Features automatic backup system and NSIS log printing
;
; |> How to use <|
; To associate a file with an application so you can double-click it in explorer, use
; the APP_ASSOCIATE macro like this:
;
;   Example:
;   !insertmacro APP_ASSOCIATE "txt" "myapp.textfile" "Description of txt files" \
;     "$INSTDIR\myapp.exe,0" "Open with myapp" "$INSTDIR\myapp.exe $\"%1$\""
;
; Never insert the APP_ASSOCIATE macro multiple times, it is only ment to associate an
; application with a single file and using the macro multiple times will just overwrite
; the previous association.
;
; To associate a file with an application and also add a "Edit with notepad" action, use
; the APP_ASSOCIATE_EX macro like this:
;
;   Example:
;   !insertmacro APP_ASSOCIATE_EX "txt" "myapp.textfile" "Description of txt files" \
;     "$INSTDIR\myapp.exe,0" "Open with myapp" "$INSTDIR\myapp.exe $\"%1$\"" \
;     "Edit with notepad" "$WINDIR\notepad.exe $\"%1$\""
;
; To unregister the file association use the APP_UNASSOCIATE macro, like this:
;
;   Example:
;   !insertmacro APP_UNASSOCIATE "txt" "myapp.textfile"
;
; If you have previously used the APP_ASSOCIATE macro, use the APP_UNASSOCIATE macro like this:
;
;   Example:
;   !insertmacro APP_UNASSOCIATE "txt" "myapp.textfile"
;
; Note that this will restore the file association that was registered before so the
; uninstaller should be able to work correctly.
;
; |> Note <|
; When defining your file class string always use the short form of your application title
; then a period (dot) then the type of file. This keeps the file class sort of unique.
;   Examples:
;     Winamp.Playlist
;     NSIS.Script
;     Photoshop.JPEGFile
;
; |> Tech info <|
; The registry key layout for a global file association is:
;
; HKEY_CLASSES_ROOT
;     <applicationID> = <"description">
;         DefaultIcon = <"path,icon_index">
;         shell
;             open = <"menu text">
;                 command = <"command string">
;             
;
; The registry key layout for a per-user file association is:
;
; HKEY_CURRENT_USER\Software\Classes
;     <applicationID> = <"description">
;         DefaultIcon = <"path,icon_index">
;         shell
;             open = <"menu text">
;                 command = <"command string">
;

!ifndef FILEASSOCIATION_NSH
!define FILEASSOCIATION_NSH

!include Util.nsh

!verbose push
!verbose 3
!ifndef _FILEASSOCIATION_VERBOSE
  !define _FILEASSOCIATION_VERBOSE 3
!endif
!verbose ${_FILEASSOCIATION_VERBOSE}
!define FILEASSOCIATION_VERBOSE `!insertmacro FILEASSOCIATION_VERBOSE`
!verbose pop

!macro FILEASSOCIATION_VERBOSE _VERBOSE
  !verbose push
  !verbose 3
  !undef _FILEASSOCIATION_VERBOSE
  !define _FILEASSOCIATION_VERBOSE ${_VERBOSE}
  !verbose pop
!macroend

!macro APP_ASSOCIATE EXT FILECLASS DESCRIPTION ICON COMMANDTEXT COMMAND
; Backup the previously associated file class
ReadRegStr $R0 HKCR ".${EXT}" ""
WriteRegStr HKCR ".${EXT}" "backup_val" "$R0"
WriteRegStr HKCR ".${EXT}" "" "${FILECLASS}"

WriteRegStr HKCR "${FILECLASS}" "" `${DESCRIPTION}`
WriteRegStr HKCR "${FILECLASS}\DefaultIcon" "" `${ICON}`
WriteRegStr HKCR "${FILECLASS}\shell" "" "open"
WriteRegStr HKCR "${FILECLASS}\shell\open" "" `${COMMANDTEXT}`
WriteRegStr HKCR "${FILECLASS}\shell\open\command" "" `${COMMAND}`
!macroend

!macro APP_ASSOCIATE_EX EXT FILECLASS DESCRIPTION ICON COMMANDTEXT COMMAND VERB VERBTEXT VERBCOMMAND
; Backup the previously associated file class
ReadRegStr $R0 HKCR ".${EXT}" ""
WriteRegStr HKCR ".${EXT}" "backup_val" "$R0"
WriteRegStr HKCR ".${EXT}" "" "${FILECLASS}"

WriteRegStr HKCR "${FILECLASS}" "" `${DESCRIPTION}`
WriteRegStr HKCR "${FILECLASS}\DefaultIcon" "" `${ICON}`
WriteRegStr HKCR "${FILECLASS}\shell" "" "open"
WriteRegStr HKCR "${FILECLASS}\shell\open" "" `${COMMANDTEXT}`
WriteRegStr HKCR "${FILECLASS}\shell\open\command" "" `${COMMAND}`
WriteRegStr HKCR "${FILECLASS}\shell\${VERB}" "" `${VERBTEXT}`
WriteRegStr HKCR "${FILECLASS}\shell\${VERB}\command" "" `${VERBCOMMAND}`
!macroend

!macro APP_UNASSOCIATE EXT FILECLASS
; Backup the previously associated file class
ReadRegStr $R0 HKCR ".${EXT}" "backup_val"
WriteRegStr HKCR ".${EXT}" "" "$R0"

DeleteRegKey HKCR `${FILECLASS}`
!macroend

!macro APP_ASSOCIATE_ADDVERB FILECLASS VERB VERBTEXT VERBCOMMAND
WriteRegStr HKCR "${FILECLASS}\shell\${VERB}" "" `${VERBTEXT}`
WriteRegStr HKCR "${FILECLASS}\shell\${VERB}\command" "" `${VERBCOMMAND}`
!macroend

!macro APP_ASSOCIATE_REMOVEVERB FILECLASS VERB
DeleteRegKey HKCR `${FILECLASS}\shell\${VERB}`
!macroend

; Simple macros for register/unregister extension
!macro registerExtension executable extension description
         ${APP_ASSOCIATE} "${extension}" "MikoIDE${extension}" "${description}" \
                         "${executable},0" "Open with MikoIDE" "${executable} $\"%1$\""
!macroend

!macro unregisterExtension extension description  
         ${APP_UNASSOCIATE} "${extension}" "MikoIDE${extension}"
!macroend

!endif ; FILEASSOCIATION_NSH