;InnoSetupVersion=5.5.0
#define DevKitVersion "1.0.11"
#define DevKitSDKVersion "0.9.5"
#define DevKitAppName "Unofficial Development Kit for Espressif ESP8266"
#define DevKitAppURL "http://www.programs74.ru"
#define DevKitAppPublisher "Mikhail Grigorev"
#define DevKitDonateEN "KEZT6SQ9FRRFE"
#define DevKitDonateRU "6D6EFK8LJ74UC"

[Setup]
AppName={#DevKitAppName}
AppVerName={#DevKitAppName}
AppCopyright=Copyright © 2014-2015 {#DevKitAppPublisher}
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
Source: "Espressif\xtensa-lx106-elf\*"; DestDir: "{app}\xtensa-lx106-elf"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\utils\*"; DestDir: "{app}\utils"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_RTOS_SDK\*"; DestDir: "{app}\ESP8266_RTOS_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_SDK\*"; DestDir: "{app}\ESP8266_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_SDK_094\*"; DestDir: "{app}\ESP8266_SDK_094"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\Espressif web site.url"; DestDir: "{app}"; Flags: ignoreversion
Source: "Espressif\DevKit web site.url"; DestDir: "{app}"; Flags: ignoreversion
Source: "Espressif\changelog.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: english
Source: "Espressif\changelog_ru.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: russian
Source: "donate-en.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: english
Source: "donate-ru.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: russian
Source: "InnoCallback.dll"; DestDir: "{tmp}"; Flags: dontcopy

[Dirs]
Name: "{app}\xtensa-lx106-elf"
Name: "{app}\docs"
Name: "{app}\utils"
Name: "{app}\examples"
Name: "{app}\ESP8266_RTOS_SDK"
Name: "{app}\ESP8266_SDK"
Name: "{app}\ESP8266_SDK_094"

[Registry]
Root: "HKCU"; Subkey: "Software\Terminal\TmacroForm"; ValueType: string; ValueName: "macro1E_Text"; ValueData: "%SCRS""C:\Espressif\utils\esp-reboot.tsc"""
Root: "HKCU"; Subkey: "Software\Terminal\TmacroForm"; ValueType: string; ValueName: "macro1NE_Text"; ValueData: "M1"

[Run]

[Icons]
Name: "{group}\Документация"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Documentation"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Примеры прошивок"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Examples"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Запустить Terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Run terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Beб-сайт Unofficial Development Kit"; Filename: "{app}\DevKit web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Unofficial Development Kit Web site"; Filename: "{app}\DevKit web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Beб-сайт Espressif"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Espressif Web site"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Сhangelog"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Список изменений"; Filename: "{app}\changelog_ru.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\{cm:UninstallProgram,Espressif ESP8266 Developer Kit}"; Filename: "{uninstallexe}"; MinVersion: 0.0,5.0

[UninstallDelete]
Type: filesandordirs; Name: "{app}\xtensa-lx106-elf\*"
Type: filesandordirs; Name: "{app}\docs\*"
Type: filesandordirs; Name: "{app}\utils\*"
Type: filesandordirs; Name: "{app}\examples\*"
Type: filesandordirs; Name: "{app}\ESP8266_RTOS_SDK\*"
Type: filesandordirs; Name: "{app}\ESP8266_SDK\*"
Type: filesandordirs; Name: "{app}\ESP8266_SDK_094\*"
Type: files; Name: "{app}\DevKit web site.url"
Type: files; Name: "{app}\Espressif web site.url"
Type: files; Name: "{app}\changelog.txt"; Languages: english
Type: files; Name: "{app}\changelog_ru.txt"; Languages: russian

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Messages]
;BeveledLabel=Espressif ESP8266 Developer Kit
russian.WelcomeLabel2=Сейчас на Ваш компьютер будет установлен {#DevKitAppName}. В состав комплекта входят неофициальный компилятор для SoC Xtensa LX106, ESP8266 SDK v{#DevKitSDKVersion}, набор утилит и примеры написания прошивок.
english.WelcomeLabel2=Now your computer will set an {#DevKitAppName}. The DevKit includes a unofficial compiler for SoC Xtensa LX106 and ESP8266 SDK v{#DevKitSDKVersion}, utils and examples of writing firmware.

[CustomMessages]
russian.UninstallProgram=Удалить %1
english.UninstallProgram=Uninstall %1
english.DONATE=Dear users, support this project and it will continue to develop:
russian.DONATE=Уважаемые пользователи, поддержите проект и благодаря Вам он сможет развиваться дальше:

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
  if ActiveLanguage = 'english' then 
    ShellExec('open', 'https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id={#DevKitDonateEN}', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode)
  else
    ShellExec('open', 'https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id={#DevKitDonateRU}', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode)
end;

procedure CreateImage;
begin
  if ActiveLanguage = 'english' then 
    ExtractTemporaryFile('donate-en.bmp')
  else
    ExtractTemporaryFile('donate-ru.bmp');
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
    if ActiveLanguage = 'english' then 
    begin
      Width:=147;
      Height:=47;
    end
    else
    begin
      Width:=92;
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
    if ActiveLanguage = 'english' then 
      Bitmap.LoadFromFile(ExpandConstant('{tmp}')+'\donate-en.bmp')
    else
      Bitmap.LoadFromFile(ExpandConstant('{tmp}')+'\donate-ru.bmp');
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
