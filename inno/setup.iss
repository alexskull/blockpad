; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "BlockPad"
#define MyAppVersion "0.7"
#define MyAppPublisher "Bloc10"
#define MyAppURL "http://www.bloc10.com/"
#define MyAppExeName "BlockPad.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{490411AD-A7AA-4C59-82B9-E8EF7650E442}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=BlockPadSetup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin
DirExistsWarning=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Registry]
Root: HKCU; Subkey: "Software\Classes\.bloc"; ValueType: string; ValueName: ""; ValueData: "_BlockPad"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\_BlockPad"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\_BlockPad\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\_BlockPad\Shell"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\_BlockPad\Shell\Open"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\_BlockPad\Shell\Open\Command"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName} ""%1"""; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\.bloc"; ValueType: string; ValueName: ""; ValueData: "_BlockPad"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\_BlockPad"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\_BlockPad\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\_BlockPad\Shell"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\_BlockPad\Shell\Open"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\_BlockPad\Shell\Open\Command"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName} ""%1"""; Flags: uninsdeletekey
[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\BlockPadBin\BlockPad.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\BlockPadBin\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent runascurrentuser

