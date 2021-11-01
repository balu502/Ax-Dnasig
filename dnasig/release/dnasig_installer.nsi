; dnasig_installer.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
;--------------------------------

!include Library.nsh


; The name of the installer
Name "DNA-Technology Signing-XML COM"

; The file to write
OutFile "Install-Dnasig.exe"

; The default installation directory
InstallDir $PROGRAMFILES\DNA-Technology\Dnasig

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\DNA-Technology\Dnasig" "Install_Dir"

; Request application privileges for Windows Vista
;RequestExecutionLevel admin
RequestExecutionLevel none

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "DNA-Tech. Dnasig COM (required)"

  SectionIn RO
   
  FindProcDLL::FindProc "dnasig.exe"
  IntCmp $R0 1 0 notRunning
	MessageBox MB_OK|MB_ICONEXCLAMATION "Dnasig is running. Please close it first" /SD IDOK
	Abort
  notRunning:
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
    
  File dnasig.exe  *.dll
  File /nonfatal *.qm  *.css  *.tlb  *.idl  *.txt
  File /r platforms
  File /r imageformats

  
  ;
  ; Grant full access for install folder
  ;    
  AccessControl::GrantOnFile "$INSTDIR" "(BU)" "FullAccess"
  Pop $R0
  ${If} $R0 == error
    Pop $R0
    DetailPrint `AccessControl error: $R0`
  ${EndIf}
  
  
  ;
  ; Register COM server
  ;  
  ClearErrors
  Exec '"$INSTDIR\dnasig.exe" -regserver'
  IfErrors 0 noRerr
	DetailPrint "Cannot register COM server Dnasig"
	Abort
  noRerr:  
  DetailPrint "Register COM server Dnasig complete"
  
  ;
  ; Write the installation path into the registry
  ;
  WriteRegStr HKLM SOFTWARE\DNA-Technology\Dnasig "Install_Dir" "$INSTDIR"
  
  ;
  ; Write the uninstall keys for Windows
  ;
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DNA-Technology\Dnasig" 	"DisplayName" "Dnasig"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DNA-Technology\Dnasig" 	"UninstallString" '"$INSTDIR\uninstall-dnasig.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DNA-Technology\Dnasig" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DNA-Technology\Dnasig" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall-dnasig.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\DNA-Technology\Dnasig"
  CreateShortcut  "$SMPROGRAMS\DNA-Technology\Dnasig\Dnasig.lnk" "$INSTDIR\dnasig.exe" "" "$INSTDIR\dnasig.exe" 0
  CreateShortcut  "$SMPROGRAMS\DNA-Technology\Dnasig\Uninstall-dnasig.lnk" "$INSTDIR\uninstall-dnasig.exe" "" "$INSTDIR\uninstall-dnasig.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
 
  FindProcDLL::FindProc "dnasig.exe"
  IntCmp $R0 1 0 notRunning
	MessageBox MB_OK|MB_ICONEXCLAMATION "Dnasig is running. Please close it first" /SD IDOK
	Abort
  notRunning: 
 
 
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DNA-Technology\Dnasig"
  DeleteRegKey HKLM "SOFTWARE\DNA-Technology\Dnasig"

  ;
  ; Unregister COM server
  ;  
  ClearErrors
  Exec '"$INSTDIR\dnasig.exe" -unregserver'
  IfErrors 0 noRerr
	DetailPrint "Cannot unregister COM server Dnasig"
	Goto continueUnsl
  noRerr:  
  DetailPrint "Unregister COM server Dnasig complete" 
  continueUnsl:
  
  ; Remove files and uninstaller
  Delete $INSTDIR\dnasig.exe
  Delete $INSTDIR\*
  
  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\DNA-Technology\Dnasig\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\DNA-Technology\Dnasig"
  
  RMDir /r "$INSTDIR"
  Sleep 3000
  RMDir /r "$INSTDIR"

SectionEnd
