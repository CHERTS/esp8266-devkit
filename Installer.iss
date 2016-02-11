;InnoSetupVersion=5.5.0
#define DevKitVersion "2.1.0"
#define DevKitSDKVersion "1.5.2"
#define DevKitAppName "Unofficial Development Kit for Espressif ESP8266"
#define DevKitAppURL "http://www.programs74.ru"
#define DevKitAppPublisher "Mikhail Grigorev"
#define DevKitDonateEN "KEZT6SQ9FRRFE"
#define DevKitDonateRU "6D6EFK8LJ74UC"
#define DevKitDonateTR "BAVDMNVFZ6WQY"

[Setup]
AppName={#DevKitAppName}
AppVerName={#DevKitAppName}
AppCopyright=Copyright © 2014-2016 {#DevKitAppPublisher}
AppContact=sleuthhound@gmail.com
AppPublisher={#DevKitAppPublisher}
AppPublisherURL={#DevKitAppURL}
AppVersion={#DevKitVersion}
AppSupportURL={#DevKitAppURL}
AppUpdatesURL={#DevKitAppURL}
DefaultDirName=C:\Espressif
DisableDirPage=True
DisableProgramGroupPage=True
AlwaysShowDirOnReadyPage=True
AlwaysShowGroupOnReadyPage=True
DefaultGroupName=Espressif
OutputBaseFilename=Espressif-ESP8266-DevKit-v{#DevKitVersion}-x86
Compression=lzma
AllowRootDirectory=True
RestartIfNeededByRun=False
;SignTool=signtool sign /f "Mikhail_Grigorev-Open_Source_Developer_2014-2015.pfx" /p ************* /t http://time.certum.pl/authenticode $f
SetupIconFile=Install.ico
UninstallDisplayIcon=Uninstall.ico
WizardImageFile=ESP8266P.bmp
LicenseFile=license.txt

[Files]
Source: "Espressif\xtensa-lx106-elf\*"; DestDir: "{app}\xtensa-lx106-elf"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: compiler
Source: "Espressif\docs\ESP8266\*"; DestDir: "{app}\docs\ESP8266"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: docs
Source: "Espressif\utils\ESP8266\*"; DestDir: "{app}\utils\ESP8266"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: utils
Source: "Espressif\utils\Terminal.exe"; DestDir: "{app}\utils"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: utils
Source: "Espressif\utils\esp-reboot.tsc"; DestDir: "{app}\utils"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: utils
Source: "Espressif\examples\ESP8266\*"; DestDir: "{app}\examples\ESP8266"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: examples
Source: "Espressif\extra\*"; DestDir: "{app}\extra"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_MESH_SDK\*"; DestDir: "{app}\ESP8266_MESH_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_RTOS_SDK\*"; DestDir: "{app}\ESP8266_RTOS_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK\*"; DestDir: "{app}\ESP8266_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_094\*"; DestDir: "{app}\ESP8266_SDK_094"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_095\*"; DestDir: "{app}\ESP8266_SDK_095"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_101\*"; DestDir: "{app}\ESP8266_SDK_101"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_110\*"; DestDir: "{app}\ESP8266_SDK_110"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_120\*"; DestDir: "{app}\ESP8266_SDK_120"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_130\*"; DestDir: "{app}\ESP8266_SDK_130"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_141\*"; DestDir: "{app}\ESP8266_SDK_141"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_150\*"; DestDir: "{app}\ESP8266_SDK_150"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\ESP8266_SDK_151\*"; DestDir: "{app}\ESP8266_SDK_151"; Flags: ignoreversion createallsubdirs recursesubdirs; Components: sdk
Source: "Espressif\Espressif web site.url"; DestDir: "{app}"; Flags: ignoreversion
Source: "Espressif\DevKit web site.url"; DestDir: "{app}"; Flags: ignoreversion
Source: "Espressif\Command line Unofficial Development Kit for Espressif ESP8266.lnk"; DestDir: "{app}"; Flags: ignoreversion; Languages: english turkish; Components: utils
Source: "Espressif\Командная строка Unofficial Development Kit for Espressif ESP8266.lnk"; DestDir: "{app}"; Flags: ignoreversion; Languages: russian; Components: utils
Source: "Espressif\changelog.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: english turkish
Source: "Espressif\changelog_ru.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: russian
Source: "Install.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "Uninstall.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "donate-en.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: english
Source: "donate-ru.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: russian
Source: "donate-tr.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: turkish
Source: "InnoCallback.dll"; DestDir: "{tmp}"; Flags: dontcopy

[Dirs]
Name: "{app}\xtensa-lx106-elf"; Components: compiler
Name: "{app}\docs\ESP8266"; Components: docs
Name: "{app}\utils\ESP8266"; Components: utils
Name: "{app}\examples\ESP8266"; Components: examples
Name: "{app}\extra"; Components: sdk
Name: "{app}\ESP8266_MESH_SDK"; Components: sdk
Name: "{app}\ESP8266_RTOS_SDK"; Components: sdk
Name: "{app}\ESP8266_SDK"; Components: sdk
Name: "{app}\ESP8266_SDK_094"; Components: sdk
Name: "{app}\ESP8266_SDK_095"; Components: sdk
Name: "{app}\ESP8266_SDK_101"; Components: sdk
Name: "{app}\ESP8266_SDK_110"; Components: sdk
Name: "{app}\ESP8266_SDK_120"; Components: sdk
Name: "{app}\ESP8266_SDK_130"; Components: sdk
Name: "{app}\ESP8266_SDK_141"; Components: sdk
Name: "{app}\ESP8266_SDK_150"; Components: sdk
Name: "{app}\ESP8266_SDK_151"; Components: sdk

[Registry]
Root: "HKCU"; Subkey: "Software\Terminal\TmacroForm"; ValueType: string; ValueName: "macro1E_Text"; ValueData: "%SCRS""C:\Espressif\utils\esp-reboot.tsc"""
Root: "HKCU"; Subkey: "Software\Terminal\TmacroForm"; ValueType: string; ValueName: "macro1NE_Text"; ValueData: "M1"

[Run]

[Icons]
Name: "{group}\Documentation"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: english; Components: docs
Name: "{group}\Документация"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: russian; Components: docs
Name: "{group}\Belgeler"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: turkish; Components: docs
Name: "{group}\Examples"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: english; Components: examples
Name: "{group}\Примеры прошивок"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: russian; Components: examples
Name: "{group}\Örnekleri firmware"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: turkish; Components: examples
Name: "{group}\Run terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: english; Components: utils
Name: "{group}\Запустить Terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: russian; Components: utils
Name: "{group}\Programı Terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: turkish; Components: utils
Name: "{group}\Command line Unofficial Development Kit for Espressif ESP8266"; Filename: "{app}\Command line Unofficial Development Kit for Espressif ESP8266.lnk"; WorkingDir: "{app}\"; MinVersion: 0.0,5.0; Languages: english turkish; Components: utils
Name: "{group}\Командная строка Unofficial Development Kit for Espressif ESP8266"; Filename: "{app}\Командная строка Unofficial Development Kit for Espressif ESP8266.lnk"; WorkingDir: "{app}\"; MinVersion: 0.0,5.0; Languages: russian; Components: utils
Name: "{group}\Espressif Web site"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Beб-сайт Espressif"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Web sitesi Espressif"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\Сhangelog"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Список изменений"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Değişikliklerin listesi"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\{cm:UninstallProgram,{#DevKitAppName}}"; Filename: "{uninstallexe}"; IconFilename: "{app}\Install.ico"; MinVersion: 0.0,5.0

[UninstallDelete]
Type: filesandordirs; Name: "{app}\xtensa-lx106-elf\*"; Components: compiler
Type: filesandordirs; Name: "{app}\docs\ESP8266\*"; Components: docs
Type: filesandordirs; Name: "{app}\utils\ESP8266\*"; Components: utils
Type: filesandordirs; Name: "{app}\examples\ESP8266\*"; Components: examples
Type: filesandordirs; Name: "{app}\extra\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_MESH_SDK\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_RTOS_SDK\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_094\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_095\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_101\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_110\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_120\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_130\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_141\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_150\*"; Components: sdk
Type: filesandordirs; Name: "{app}\ESP8266_SDK_151\*"; Components: sdk
Type: files; Name: "{app}\Command line Unofficial Development Kit for Espressif ESP8266.lnk"; Languages: english turkish; Components: utils
Type: files; Name: "{app}\Командная строка Unofficial Development Kit for Espressif ESP8266.lnk"; Languages: russian; Components: utils
Type: files; Name: "{app}\DevKit web site.url"
Type: files; Name: "{app}\Espressif web site.url"
Type: files; Name: "{app}\changelog.txt"; Languages: english turkish
Type: files; Name: "{app}\changelog_ru.txt"; Languages: russian
Type: files; Name: "{app}\Install.ico"
Type: files; Name: "{app}\Uninstall.ico"

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"

[Messages]
;BeveledLabel=Espressif ESP8266 Developer Kit
english.WelcomeLabel2=Now on your PC will be installed {#DevKitAppName}. The DevKit includes a unofficial compiler for SoC Xtensa LX106 and ESP8266 SDK v{#DevKitSDKVersion}, utilities and examples of writing firmware.
russian.WelcomeLabel2=Сейчас на Ваш компьютер будет установлен {#DevKitAppName}. В состав комплекта входят неофициальный компилятор для SoC Xtensa LX106, ESP8266 SDK v{#DevKitSDKVersion}, набор утилит и примеры написания прошивок.
turkish.WelcomeLabel2=Bu kurulum paketi ile bilgisayarınıza {#DevKitAppName} yüklenecek . Bu paket SoC Xtensa LX106 için derleyici, ESP8266 SDK v{#DevKitSDKVersion} ve yardımcı örnekler içerir.

[CustomMessages]
english.UninstallProgram=Uninstall %1
russian.UninstallProgram=Удалить %1
turkish.UninstallProgram=Kaldır %1
english.DONATE=Dear users, support this project and it will continue to develop:
russian.DONATE=Уважаемые пользователи, поддержите проект и благодаря Вам он сможет развиваться дальше:
turkish.DONATE=Sevgili kullanıcılar, çalışmayı beğendiyseniz lütfen bağış yaparak çalışmamın gelişimine katkıda bulunun.:
english.CUSTOMINSTALL=Custom installation
russian.CUSTOMINSTALL=Выборочная установка
turkish.CUSTOMINSTALL=Özel yükleme

;Flags: iscustom можно поставить только у одного типа, поэтому ниже на Pascal Script написан хак, как локализовать Custom installation 
[Types]
Name: "full"; Description: "Full installation (Compiler, SDK, utils, docs and examples)"; Languages: english
Name: "compact"; Description: "Compact installation (Compiler, SDK, and utils)"; Languages: english
Name: "full"; Description: "Полная установка (Компилятор, утилиты, документация и примеры)"; Languages: russian
Name: "compact"; Description: "Компактная установка (Компилятор и утилиты)"; Languages: russian
Name: "full"; Description: "Tam kurulum (Derleyici, SDK, utils, dokümanlar ve örnekler)"; Languages: turkish
Name: "compact"; Description: "Kompakt kurulum (Derleyici, SDK ve utils)"; Languages: turkish
Name: "custom"; Description: "Custom installation"; Flags: iscustom

;Flags: fixed - значит нельзя отключить!
;Flags: exclusive - можно выбрать только одно из exclusive

[Components]
Name: compiler; Description: GCC Xtensa LX106; Types: full compact custom
Name: sdk; Description: Espressif SDK; Types: full compact custom; Languages: english
Name: docs; Description: Documentation; Types: full custom; Languages: english
Name: utils; Description: Utilites; Types: full compact custom; Languages: english
Name: examples; Description: Examples; Types: full custom; Languages: english
Name: sdk; Description: "Espressif SDK"; Types: full compact custom; Languages: russian
Name: docs; Description: "Документация"; Types: full custom; Languages: russian
Name: utils; Description: "Утилиты"; Types: full compact custom; Languages: russian
Name: examples; Description: "Примеры"; Types: full custom; Languages: russian
Name: sdk; Description: Espressif SDK; Types: full compact custom; Languages: turkish
Name: docs; Description: "Doküman"; Types: full custom; Languages: turkish
Name: utils; Description: "Utilites"; Types: full compact custom; Languages: turkish
Name: examples; Description: "Örnekler"; Types: full custom; Languages: turkish
;Name: "readme"; Description: "Readme File"; Types: full
;Name: "readme\en"; Description: "English"; Flags: exclusive
;Name: "readme\ru"; Description: "Russian"; Flags: exclusive

[Code]
const
  SB_VERT = 1;
  SIF_RANGE = 1;
  SIF_POS = 4;
  SIF_PAGE = 2;

type
  TScrollInfo = record
    cbSize: UINT;
    fMask: UINT;
    nMin: Integer;
    nMax: Integer;
    nPage: UINT;
    nPos: Integer;
    nTrackPos: Integer;
  end;
  TTimerProc = procedure(Wnd: HWND; Msg: UINT; TimerID: UINT_PTR; SysTime: DWORD);

var
  StaticText: TNewStaticText;
  DonatePanel: TPanel;
  DonateImage: TBitmapImage;
  TimerID: Integer;

function GetScrollInfo(hWnd: HWND; BarFlag: Integer; var ScrollInfo: TScrollInfo): BOOL; external 'GetScrollInfo@user32.dll stdcall';
function WrapTimerProc(Callback: TTimerProc; ParamCount: Integer): LongWord; external 'wrapcallback@files:InnoCallback.dll stdcall';    
function SetTimer(hWnd: HWND; nIDEvent, uElapse: UINT; lpTimerFunc: UINT): UINT; external 'SetTimer@user32.dll stdcall';
function KillTimer(hWnd: HWND; uIDEvent: UINT): BOOL; external 'KillTimer@user32.dll stdcall'; 
     
procedure SiteLabelOnClick(Sender: TObject);
var
  ErrorCode: Integer;
begin
  if ActiveLanguage = 'russian' then 
    ShellExec('open', 'https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id={#DevKitDonateRU}', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode)
  else if ActiveLanguage = 'turkish' then 
    ShellExec('open', 'https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id={#DevKitDonateTR}', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode)
  else
    ShellExec('open', 'https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id={#DevKitDonateEN}', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode)
end;

procedure CreateImage;
begin
  if ActiveLanguage = 'russian' then 
    ExtractTemporaryFile('donate-ru.bmp')
  else if ActiveLanguage = 'turkish' then 
    ExtractTemporaryFile('donate-tr.bmp')
  else
    ExtractTemporaryFile('donate-en.bmp');
  StaticText:=TNewStaticText.Create(WizardForm);
  with StaticText do
  begin
    Left:=176
    Top:=220
    Width:=300;
    Height:=23;
    WordWrap:=True;
    Color:=clWhite;
    Caption:=ExpandConstant('{cm:DONATE}')
    Parent:=WizardForm;
  end;
  DonatePanel:=TPanel.Create(WizardForm);
  with DonatePanel do
  begin
    Left:=176;
    Top:=250;
    if ActiveLanguage = 'russian' then 
    begin
      Width:=92;
      Height:=47;
    end
    else if ActiveLanguage = 'turkish' then 
    begin
      Width:=129;
      Height:=47;
    end
    else
    begin
      Width:=147;
      Height:=47;
    end;
    Cursor:=crHand;
    OnClick:=@SiteLabelOnClick;
    Parent:=WizardForm;
  end;
  DonateImage:=TBitmapImage.Create(WizardForm);
  with DonateImage do
  begin
    AutoSize:=True;
    Enabled:=False;
    if ActiveLanguage = 'russian' then 
      Bitmap.LoadFromFile(ExpandConstant('{tmp}')+'\donate-ru.bmp')
    else if ActiveLanguage = 'turkish' then 
      Bitmap.LoadFromFile(ExpandConstant('{tmp}')+'\donate-tr.bmp')
    else
      Bitmap.LoadFromFile(ExpandConstant('{tmp}')+'\donate-en.bmp');
    Parent:=DonatePanel;
  end;
end;

procedure OnScrollPosition(Wnd: HWND; Msg: UINT; TimerID: UINT_PTR; SysTime: DWORD);
var
  ScrollInfo: TScrollInfo;
begin
  ScrollInfo.cbSize := SizeOf(ScrollInfo);
  ScrollInfo.fMask := SIF_RANGE or SIF_POS or SIF_PAGE;
  if GetScrollInfo(WizardForm.LicenseMemo.Handle, SB_VERT, ScrollInfo) then
    if ScrollInfo.nPos = ScrollInfo.nMax - ScrollInfo.nPage then
    begin
      WizardForm.LicenseAcceptedRadio.Enabled := True;
      WizardForm.LicenseNotAcceptedRadio.Enabled := True;
    end;
end;

procedure StartSlideTimer();
var
  TimerCallback: LongWord;
begin
  TimerCallback := WrapTimerProc(@OnScrollPosition, 4);
  TimerID := SetTimer(0, 0, 500, TimerCallback);
end;

procedure KillSlideTimer;
begin
  if TimerID <> 0 then 
  begin
    if KillTimer(0, TimerID) then
      TimerID := 0;
  end;
end;

procedure CurPageChanged(CurPageID: Integer);
var
  OptionsPage: TWizardPage;
  PageSurface: TNewNotebookPage;
  SetupTypesCombo: TComboBox;
begin
{  if CurPageID = wpLicense then
  begin
    WizardForm.LicenseAcceptedRadio.Enabled := False;
    WizardForm.LicenseNotAcceptedRadio.Enabled := False;
    StartSlideTimer;
  end
  else if CurPageID = wpFinished then
}
  if CurPageID = wpFinished then
    CreateImage()
  // Хак для локализации Custom installation
  else if (CurPageID = wpSelectComponents) then
  begin
    OptionsPage := PageFromID(CurPageID);
    PageSurface := OptionsPage.Surface;
    SetupTypesCombo := TComboBox(PageSurface.Controls[2]);
    if (SetupTypesCombo <> nil) then
    begin
      if (SetupTypesCombo.Items[2] <> ExpandConstant('{cm:CUSTOMINSTALL}')) then
      begin
        SetupTypesCombo.Items[2] := ExpandConstant('{cm:CUSTOMINSTALL}');
      end;
    end;
  end
  else
  begin
    {KillSlideTimer;}
    if Assigned(StaticText) then
    begin
      StaticText.Free;
      StaticText:=nil;
    end;   
    if Assigned(DonateImage) then
    begin
      DonateImage.Free;
      DonateImage:=nil;
    end;
    if Assigned(DonatePanel) then
    begin
      DonatePanel.Free;
      DonatePanel:=nil;
    end;
  end;
end;

function GetUninstallString: string;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  Result := '';
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#DevKitAppName}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;

function IsUpgrade: Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

function InitializeSetup: Boolean;
var
  V: Integer;
  iResultCode: Integer;
  sUnInstallString: string;
begin
  Result := True;
  if RegValueExists(HKEY_LOCAL_MACHINE,'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#DevKitAppName}_is1', 'UninstallString') then
  begin
    if ActiveLanguage = 'russian' then 
      V := MsgBox(ExpandConstant('Была найдена более старая версия {#DevKitAppName}. Для продолжения установки старую версию необходимо удалить. Согласны?'), mbInformation, MB_YESNO)
    else
      V := MsgBox(ExpandConstant('An old version of {#DevKitAppName} was detected. Do you want to uninstall it?'), mbInformation, MB_YESNO);
    if V = IDYES then
    begin
      sUnInstallString := GetUninstallString();
      sUnInstallString :=  RemoveQuotes(sUnInstallString);
      Exec(ExpandConstant(sUnInstallString), '', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
      Result := True; // Продолжение установки новой версии после удаления старой
      //Exit; // Выход из установки новой версии после удаления старой
    end
    else
      Result := False; // Если старая версия не удалена, то выход из инсталлятора
  end;
end;
