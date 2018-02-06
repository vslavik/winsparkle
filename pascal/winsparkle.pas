{
   *  This file is part of WinSparkle (https://winsparkle.org)
   *
   *  Copyright (C) 2009-2016 Theodore Tsirpanis
   *
   *  Permission is hereby granted, free of charge, to any person obtaining a
   *  copy of this software and associated documentation files (the "Software"),
   *  to deal in the Software without restriction, including without limitation
   *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
   *  and/or sell copies of the Software, and to permit persons to whom the
   *  Software is furnished to do so, subject to the following conditions:
   *
   *  The above copyright notice and this permission notice shall be included in
   *  all copies or substantial portions of the Software.
   *
   *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   *  DEALINGS IN THE SOFTWARE.
   *
 }
unit winsparkle;

{$MODE OBJFPC}{$H+}
{$IFNDEF MSWINDOWS}
{$FATAL WinSparkle requires Windows}
{$ENDIF}

interface

uses
  ctypes;

const
  WinSparkleLib = 'WinSparkle.dll';

type
  TUpdateType = (utSilent, utWithUI, utWithUIAndInstall);
  TCanShutDownCallback = function: boolean;

  { TWinSparkle }

  TWinSparkle = class
  private
    class var
    FOnCancelledUpdate, FOnError, FOnShutdown, FOnUpdateFound,
    FOnUpdateNotFound: TProcedure;
    class var
    FOnCanShutDown: TCanShutDownCallback;
    class function GetAutoCheck: boolean; static;
    class function GetAutoCheckIntereval: cint; static;
    class function GetLastCheckTime: TDateTime; static;
    class procedure SetAppcast(AValue: string); static;
    class procedure SetDSAPubPem(AValue: string); static;
    class procedure SetAutoCheck(AValue: boolean); static;
    class procedure SetAutoCheckIntereval(AValue: cint); static;
    class procedure SetBuildVersion(AValue: UnicodeString); static;
    class procedure SetLangID(AValue: cushort); static;
    class procedure SetLanguage(AValue: string); static;
    class procedure SetRegistryPath(AValue: string); static;
  public
    class procedure Init;
    class procedure Cleanup;
    class property Appcast: string write SetAppcast;
    class property DSAPubPem: string write SetDSAPubPem;
    class property Language: string write SetLanguage;
    class property LangID: cushort write SetLangID;
    class procedure SetAppDetails(const CompanyName, AppName, AppVersion: UnicodeString);
    class property BuildVersion: UnicodeString write SetBuildVersion;
    class property RegistryPath: string write SetRegistryPath;
    class property AutoCheck: boolean read GetAutoCheck write SetAutoCheck;
    class property AutoCheckIntereval: cint read GetAutoCheckIntereval
      write SetAutoCheckIntereval;
    class property LastCheckTime: TDateTime read GetLastCheckTime;
    class property OnError: TProcedure read FOnError write FOnError;
    class property OnCanShutdown: TCanShutDownCallback
      read FOnCanShutDown write FOnCanShutDown;
    class property OnShutdown: TProcedure read FOnShutdown write FOnShutdown;
    class property OnUpdateFound: TProcedure read FOnUpdateFound write FOnUpdateFound;
    class property OnUpdateNotFound: TProcedure read FOnUpdateNotFound
      write FOnUpdateNotFound;
    class property OnCancelledUpdate: TProcedure
      read FOnCancelledUpdate write FOnCancelledUpdate;
    class procedure CheckUpdates(const UpdateType: TUpdateType);
  end;



implementation

uses dateutils;

type
  TWinSparkleCallbackVoid = procedure; cdecl;
  TWinSparkleCallbackCanShutdown = function: longbool; cdecl;

procedure win_sparkle_init; cdecl; external WinSparkleLib;
procedure win_sparkle_cleanup; cdecl; external WinSparkleLib;
procedure win_sparkle_set_lang(lang: PChar); cdecl; external WinSparkleLib;
procedure win_sparkle_set_langid(lang: cushort); cdecl; external WinSparkleLib;
procedure win_sparkle_set_appcast_url(url: PChar); cdecl; external WinSparkleLib;
procedure win_sparkle_set_dsa_pub_pem(pem: PChar); cdecl; external WinSparkleLib;
procedure win_sparkle_set_app_details(company_name, app_name, app_version: PWideChar);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_app_build_version(build: PWideChar);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_registry_path(path: PChar); cdecl; external WinSparkleLib;
procedure win_sparkle_set_automatic_check_for_updates(state: longbool);
  cdecl; external WinSparkleLib;
function win_sparkle_get_automatic_check_for_updates: longbool;
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_update_check_interval(interval: cint);
  cdecl; external WinSparkleLib;
function win_sparkle_get_update_check_interval: cint; cdecl; external WinSparkleLib;
function win_sparkle_get_last_check_time: clong; cdecl;
  external WinSparkleLib;//Originally, it returns a time_t
procedure win_sparkle_set_error_callback(callback: TWinSparkleCallbackVoid);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_can_shutdown_callback(callback:
  TWinSparkleCallbackCanShutdown);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_shutdown_request_callback(callback: TWinSparkleCallbackVoid);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_did_find_update_callback(callback: TWinSparkleCallbackVoid);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_did_not_find_update_callback(callback:
  TWinSparkleCallbackVoid);
  cdecl; external WinSparkleLib;
procedure win_sparkle_set_update_cancelled_callback(callback: TWinSparkleCallbackVoid);
  cdecl; external WinSparkleLib;
procedure win_sparkle_check_update_with_ui; cdecl; external WinSparkleLib;
procedure win_sparkle_check_update_with_ui_and_install; cdecl; external WinSparkleLib;
procedure win_sparkle_check_update_without_ui; cdecl; external WinSparkleLib;

procedure DoOnError; cdecl;
begin
  if Assigned(TWinSparkle.FOnError) then
    TWinSparkle.FOnError();
end;

procedure DoOnShutdown; cdecl;
begin
  if Assigned(TWinSparkle.FOnShutdown) then
    TWinSparkle.FOnShutdown();
end;

procedure DoOnCancelledUpdate; cdecl;
begin
  if Assigned(TWinSparkle.FOnCancelledUpdate) then
    TWinSparkle.FOnCancelledUpdate();
end;

procedure DoOnUpdateFound; cdecl;
begin
  if Assigned(TWinSparkle.FOnUpdateFound) then
    TWinSparkle.FOnUpdateFound();
end;

procedure DoOnUpdateNotFound; cdecl;
begin
  if Assigned(TWinSparkle.FOnUpdateNotFound) then
    TWinSparkle.FOnUpdateNotFound();
end;

function DoOnCanShutDown: longbool; cdecl;
begin
  if Assigned(TWinSparkle.FOnCanShutDown) then
    Result := TWinSparkle.FOnCanShutDown()
  else
    Result := True;
end;


{ TWinSparkle }

class function TWinSparkle.GetAutoCheck: boolean; static;
begin
  Result := boolean(win_sparkle_get_automatic_check_for_updates);
end;

class function TWinSparkle.GetAutoCheckIntereval: cint; static;
begin
  Result := win_sparkle_get_update_check_interval;
end;

class function TWinSparkle.GetLastCheckTime: TDateTime; static;
begin
  Result := UnixToDateTime(win_sparkle_get_last_check_time);
end;

class procedure TWinSparkle.SetAppcast(AValue: string); static;
begin
  win_sparkle_set_appcast_url(PChar(Utf8ToAnsi(AValue)));
end;

class procedure TWinSparkle.SetDSAPubPem(AValue: string); static;
begin
  win_sparkle_set_dsa_pub_pem(PChar(Utf8ToAnsi(AValue)));
end;

class procedure TWinSparkle.SetAutoCheck(AValue: boolean); static;
begin
  win_sparkle_set_automatic_check_for_updates(AValue);
end;

class procedure TWinSparkle.SetAutoCheckIntereval(AValue: cint); static;
begin
  win_sparkle_set_update_check_interval(AValue);
end;

class procedure TWinSparkle.SetBuildVersion(AValue: UnicodeString);
begin
  win_sparkle_set_app_build_version(PWideChar(AValue));
end;

class procedure TWinSparkle.SetLangID(AValue: cushort); static;
begin
  win_sparkle_set_langid(AValue);
end;

class procedure TWinSparkle.SetLanguage(AValue: string); static;
begin
  win_sparkle_set_lang(PChar(AValue));
end;

class procedure TWinSparkle.SetRegistryPath(AValue: string); static;
begin
  win_sparkle_set_registry_path(PChar(AValue));
end;

class procedure TWinSparkle.Init;
begin
  win_sparkle_set_can_shutdown_callback(@DoOnCanShutDown);
  win_sparkle_set_did_find_update_callback(@DoOnUpdateFound);
  win_sparkle_set_did_not_find_update_callback(@DoOnUpdateNotFound);
  win_sparkle_set_shutdown_request_callback(@DoOnShutdown);
  win_sparkle_set_error_callback(@DoOnError);
  win_sparkle_set_update_cancelled_callback(@DoOnCancelledUpdate);
  win_sparkle_init;
end;

class procedure TWinSparkle.Cleanup;
begin
  win_sparkle_cleanup;
end;

class procedure TWinSparkle.SetAppDetails(
  const CompanyName, AppName, AppVersion: UnicodeString);
begin
  win_sparkle_set_app_details(PWideChar(CompanyName), PWideChar(AppName),
    PWideChar(AppVersion));
end;

class procedure TWinSparkle.CheckUpdates(const UpdateType: TUpdateType);
begin
  case UpdateType of
    utSilent: win_sparkle_check_update_without_ui;
    utWithUI: win_sparkle_check_update_with_ui;
    utWithUIAndInstall: win_sparkle_check_update_with_ui_and_install;
  end;
end;

end.
