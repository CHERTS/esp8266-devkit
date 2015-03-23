;InnoSetupVersion=5.5.0
#define DevKitVersion "2.0.0"
#define DevKitSDKVersion "1.0.0"
#define DevKitAppName "Unofficial Development Kit for Espressif ESP8266"
#define DevKitAppURL "http://www.programs74.ru"
#define DevKitAppPublisher "Mikhail Grigorev"
#define DevKitDonateEN "KEZT6SQ9FRRFE"
#define DevKitDonateRU "6D6EFK8LJ74UC"
#define DevKitDonateTR "BAVDMNVFZ6WQY"

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
Source: "Espressif\extra\*"; DestDir: "{app}\extra"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_RTOS_SDK\*"; DestDir: "{app}\ESP8266_RTOS_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_SDK\*"; DestDir: "{app}\ESP8266_SDK"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_SDK_094\*"; DestDir: "{app}\ESP8266_SDK_094"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\ESP8266_SDK_095\*"; DestDir: "{app}\ESP8266_SDK_095"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "Espressif\Espressif web site.url"; DestDir: "{app}"; Flags: ignoreversion
Source: "Espressif\DevKit web site.url"; DestDir: "{app}"; Flags: ignoreversion
Source: "Espressif\changelog.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: english turkish
Source: "Espressif\changelog_ru.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: russian
Source: "donate-en.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: english
Source: "donate-ru.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: russian
Source: "donate-tr.bmp"; DestDir: "{tmp}"; Flags: dontcopy; Languages: turkish
Source: "InnoCallback.dll"; DestDir: "{tmp}"; Flags: dontcopy

[Dirs]
Name: "{app}\xtensa-lx106-elf"
Name: "{app}\docs"
Name: "{app}\utils"
Name: "{app}\examples"
Name: "{app}\extra"
Name: "{app}\ESP8266_RTOS_SDK"
Name: "{app}\ESP8266_SDK"
Name: "{app}\ESP8266_SDK_094"
Name: "{app}\ESP8266_SDK_095"

[Registry]
Root: "HKCU"; Subkey: "Software\Terminal\TmacroForm"; ValueType: string; ValueName: "macro1E_Text"; ValueData: "%SCRS""C:\Espressif\utils\esp-reboot.tsc"""
Root: "HKCU"; Subkey: "Software\Terminal\TmacroForm"; ValueType: string; ValueName: "macro1NE_Text"; ValueData: "M1"

[Run]

[Icons]
Name: "{group}\Documentation"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Документация"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Belgeler"; Filename: "{app}\docs"; WorkingDir: "{app}\docs"; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\Examples"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Примеры прошивок"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Örnekleri firmware"; Filename: "{app}\examples"; WorkingDir: "{app}\examples"; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\Run terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Запустить Terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Programı Terminal"; Filename: "{app}\utils\Terminal.exe"; WorkingDir: "{app}\utils\"; IconFilename: "{app}\utils\Terminal.exe"; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\Espressif Web site"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Beб-сайт Espressif"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Web sitesi Espressif"; Filename: "{app}\Espressif web site.url"; WorkingDir: "{app}"; IconFilename: "%SystemRoot%\system32\SHELL32.dll"; IconIndex: 13; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\Сhangelog"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: english
Name: "{group}\Список изменений"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: russian
Name: "{group}\Değişikliklerin listesi"; Filename: "{app}\changelog.txt"; WorkingDir: "{app}"; MinVersion: 0.0,5.0; Languages: turkish
Name: "{group}\{cm:UninstallProgram,Espressif ESP8266 Developer Kit}"; Filename: "{uninstallexe}"; MinVersion: 0.0,5.0

[UninstallDelete]
Type: filesandordirs; Name: "{app}\xtensa-lx106-elf\*"
Type: filesandordirs; Name: "{app}\docs\*"
Type: filesandordirs; Name: "{app}\utils\*"
Type: filesandordirs; Name: "{app}\examples\*"
Type: filesandordirs; Name: "{app}\extra\*"
Type: filesandordirs; Name: "{app}\ESP8266_RTOS_SDK\*"
Type: filesandordirs; Name: "{app}\ESP8266_SDK\*"
Type: filesandordirs; Name: "{app}\ESP8266_SDK_094\*"
Type: filesandordirs; Name: "{app}\ESP8266_SDK_095\*"
Type: files; Name: "{app}\DevKit web site.url"
Type: files; Name: "{app}\Espressif web site.url"
Type: files; Name: "{app}\changelog.txt"; Languages: english turkish
Type: files; Name: "{app}\changelog_ru.txt"; Languages: russian

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"

[Messages]
;BeveledLabel=Espressif ESP8266 Developer Kit
english.WelcomeLabel2=Now on your PC will be installed {#DevKitAppName}. The DevKit includes a unofficial compiler for SoC Xtensa LX106 and ESP8266 SDK v{#DevKitSDKVersion}, utilities and examples of writing firmware.
russian.WelcomeLabel2=Сейчас на Ваш компьютер будет установлен {#DevKitAppName}. В состав комплекта входят неофициальный компилятор для SoC Xtensa LX106, ESP8266 SDK v{#DevKitSDKVersion}, набор утилит и примеры написания прошивок.
turkish.WelcomeLabel2=Şu an bilgisayarınızda yüklü olacak {#DevKitAppName}. Kompozisyon kit içerir derleyici için SoC Xtensa LX106, ESP8266 SDK v{#DevKitSDKVersion}, yardımcı ve örnekleri.

[CustomMessages]
english.UninstallProgram=Uninstall %1
russian.UninstallProgram=Удалить %1
turkish.UninstallProgram=Kaldır %1
english.DONATE=Dear users, support this project and it will continue to develop:
russian.DONATE=Уважаемые пользователи, поддержите проект и благодаря Вам он сможет развиваться дальше:
turkish.DONATE=Sevgili kullanıcılar, lütfen yardım projesi:

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
