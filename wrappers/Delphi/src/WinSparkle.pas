{
   *  This file is part of WinSparkle (https://winsparkle.org)
   *
   *  Copyright (C) 2016 John Kouraklis
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
   *
   *  This unit is inspired by the Free Pascal/Lazarus wrapper
   *  written by Theodore Tsirpanis
   *
   *  Special thanks to Ondrej Kelle and David Heffernan who
   *  helped me understand a few details regarding the use of DLLs
   *  in Delphi
   *

 }

{$REGION 'A Delphi wrapper for the WinSparkle project. This is a FireMonkey unit.'}
/// <summary>
///   <para>
///     A Delphi wrapper for the <see href="https://winsparkle.org">
///     WinSparkle</see> project.
///   </para>
///   <para>
///     This is a FireMonkey unit.
///   </para>
/// </summary>
{$ENDREGION}
unit WinSparkle;

interface

uses
  System.SysUtils;

const
  WinSparkleLib = 'WinSparkle.dll';

  MajorVersion = '1';
  MinorVersion = '0';
  BugVersion = '0';

//***************************************************************
//
// Version History
//
//
//
// 1.0.0 - 29/07/2016 - (Win32, Win64)
//
// ** Initial Release
//
//    * WinSparkle version: 0.5.2
//
//***************************************************************


type
  TUpdateType = (utWithoutUI, utWithUI, utWithUIAndInstall);
  TCallbackType = (cbError, cbShutdown, cbDidFindUpdate,
                    cbDidNotFindUpdate, cbUpdateCancelled);
  TCallbackTypeFunc = (cbCanShutdown);
  TCallbackArray = array[cbError..cbUpdateCancelled] of TProc;
  TCallbackArrayFunctions = array[cbCanShutdown..cbCanShutdown] of TFunc<Boolean>;

  { TWinSparkle }

  TWinSparkle = class
  private
    fHandle: THandle;
    fDLLPath: string;

    fCompanyName,
    fAppName,
    fAppVersion,
    fAppcastURL,
    fAppBuildVersion,
    fRegistryPath: string;

    fOnError,
    fOnShutDown,
    fOnDidFindUpdate,
    fOnDidNotFindUpdate,
    fOnUpdateCancelled: TProc;

    fOnCanShutDown: TFunc<Boolean>;

    function getDLLLoaded: Boolean;
    procedure ExecuteSimpleProcedure(const procName: PWideChar);
    procedure ExecuteReferencedProcedure (const procName: PWideChar;
      const procType: TCallbackType; const newProc: TProc);
    procedure ExecuteReferencedFunctionBoolean (const funcName: PWideChar;
      const funcType: TCallbackTypeFunc; const newFunc: TFunc<Boolean>);

    procedure SetAutoCheckForUpdates (const autoCheck: Boolean);
    function GetAutoCheckForUpdates: Boolean;

    function GetLastCheckTime: TDateTime;

    procedure SetAppcastURL(const newURL: string);
    function GetAppcastURL: string;

    procedure SetAppBuildVersion (const newAppBuildVersion: string);
    function GetAppBuildVersion: string;

    procedure SetRegistryPath(const newPath: string);
    function GetRegistryPath: string;

    procedure SetUpdateCheckInterval(const newInterval: Integer);
    function GetUpdateCheckInterval: Integer;

    procedure SetOnError (const newProc: TProc);
    procedure SetOnShutDown (const newProc: TProc);
    procedure SetOnDidFindUpdate (const newProc: TProc);
    procedure SetOnDidNotFindUpdate (const newProc: TProc);
    procedure SetOnUpdateCancelled (const newProc: TProc);
    procedure SetOnCanShutDown (const newFunc: TFunc<Boolean>);

  public
    constructor Create; overload;
    {$REGION 'Creates a new class and passes the full path of the DLL. If newPath is empty, the DLL path is the directory of the executable'}
    /// <summary>
    ///   Creates a new class and passes the full path of the DLL. If newPath
    ///   is empty, the DLL path is the directory of the executable
    /// </summary>
    /// <param name="newPath">
    ///   The full path of the DLL
    /// </param>
    {$ENDREGION}
    constructor Create(const newPath: string); overload;
    destructor Destroy; override;
    {$REGION 'Initialises WinSparkle.'}
    /// <summary>
    ///   Initialises WinSparkle.
    /// </summary>
    /// <remarks>
    ///   According to the author, you can setup any parameters and then
    ///   execute Init.
    /// </remarks>
    {$ENDREGION}
    procedure Init;
    procedure Cleanup;
    {$REGION 'Show a window with release notes and provides choices before downloading the updates.'}
    /// <summary>
    ///   Show a window with release notes and provides choices before
    ///   downloading the updates.
    /// </summary>
    {$ENDREGION}
    procedure CheckUpdateWithUI;
    {$REGION 'Same as CheckUpdateWithUI but installs automatically the downloaded file.'}
    /// <summary>
    ///   Same as <see cref="WinSparkle|TWinSparkle.CheckUpdateWithUI">
    ///   CheckUpdateWithUI</see> but installs automatically the downloaded
    ///   file.
    /// </summary>
    {$ENDREGION}
    procedure CheckUpdateWithUIandInstall;
    {$REGION 'Doesn''t show the window with the release notes. It just downloads the file and executes it.'}
    /// <summary>
    ///   Doesn't show the window with the release notes. It just downloads the
    ///   file and executes it.
    /// </summary>
    {$ENDREGION}
    procedure CheckUpdateWithoutUI;

    {$REGION 'Sets the application details.'}
    /// <summary>
    ///   Sets the application details.
    /// </summary>
    /// <param name="companyName">
    ///   The company name
    /// </param>
    /// <param name="appName">
    ///   The application name
    /// </param>
    /// <param name="appVersion">
    ///   The application version
    /// </param>
    /// <remarks>
    ///   <list type="bullet">
    ///     <item>
    ///       Normally, this is the first procedure to be called <br /><br />
    ///     </item>
    ///     <item>
    ///       The parameters are used to create the registry path. You can
    ///       change it using <see cref="WinSparkle|TWinSparkle.SetRegistryPath(string)">
    ///       SetRegistryPath</see>
    ///     </item>
    ///   </list>
    /// </remarks>
    {$ENDREGION}
    procedure SetAppDetails(const companyName, appName, appVersion: string);
    {$REGION 'Resets the check interval to the default value (86400 secs)'}
    /// <summary>
    ///   Resets the check interval to the default value (86400 secs)
    /// </summary>
    {$ENDREGION}
    procedure ResetUpdateCheckInterval;
    {$REGION 'Sets the language of the UI.'}
    /// <summary>
    ///   Sets the language of the UI.
    /// </summary>
    /// <remarks>
    ///   <list type="bullet">
    ///     <item>
    ///       If you want to change the language at runtime, you need to
    ///       call <see cref="WinSparkle|TWinSparkle.Init">Init</see> and
    ///       reapply any settings of the event (callback functions)
    ///     </item>
    ///     <item>
    ///       <input class="note" type="hidden" value="note" /><table><tbody><tr><th><img src="images\note.png" />
    ///              Available Translations (WinSparkle v.0.5.2)</th></tr><tr><td>
    ///             ar <br />bg <br />bs <br />ca_ES <br />co <br />
    ///             cs <br />da <br />de <br />el <br />es <br />eu <br />
    ///             fr <br />fy_NL <br />he <br />hr <br />hu <br />
    ///             hy <br />id <br />it <br />ja <br />ko <br />nb <br />
    ///             nl <br />pl <br />pt_BR <br />pt_PT <br />ru <br />
    ///             sk <br />sr <br />sv <br />tr <br />uk <br />
    ///             zh_CN <br />zh_TW</td></tr></tbody></table>
    ///     </item>
    ///   </list>
    /// </remarks>
    {$ENDREGION}
    procedure SetLang(const newLang: string);
    {$REGION 'Sets the language using the LanguageID.'}
    /// <summary>
    ///   Sets the language using the LanguageID.
    /// </summary>
    {$ENDREGION}
    procedure SetLangID(const newLandID: ShortInt);
    {$REGION 'Calls the appropriate check update method depending on the updateType.'}
    /// <summary>
    ///   Calls the appropriate check update method depending on the <see cref="WinSparkle|TUpdateType">
    ///   updateType</see>.
    /// </summary>
    {$ENDREGION}
    procedure CheckUpdate(const updateType: TUpdateType);

    {$REGION 'The path of the DLL.'}
    /// <summary>
    ///   The path of the DLL.
    /// </summary>
    /// <remarks>
    ///   If empty, the path is the executable file's location.
    /// </remarks>
    {$ENDREGION}
    property DLLPath: string read fDLLPath write fDLLPath;
    {$REGION 'Checks if the DLL is loaded.'}
    /// <summary>
    ///   Checks if the DLL is loaded.
    /// </summary>
    {$ENDREGION}
    property DLLLoaded: boolean read getDLLLoaded;

    property AutoCheckForUpdates: boolean read GetAutoCheckForUpdates
      write SetAutoCheckForUpdates;
    property LastCheckTime: TDateTime read GetLastCheckTime;
    {$REGION 'This property defines the full URL path for the appcast xml file.'}
    /// <summary>
    ///   This property defines the full URL path for the appcast xml file.
    /// </summary>
    /// <remarks>
    ///   It is strongly advisable to use https connection.
    /// </remarks>
    {$ENDREGION}
    property AppcastURL: string read GetAppcastURL write SetAppcastURL;
    property AppBuildVersion: string read GetAppBuildVersion
      write SetAppBuildVersion;
    property RegistryPath: string read GetRegistryPath write SetRegistryPath;
    property UpdateCheckInterval: integer read GetUpdateCheckInterval
      write SetUpdateCheckInterval;
    {$REGION 'This event is triggered when WinSparkle gets an error while it is attempting to check for updates.'}
    /// <summary>
    ///   This event is triggered when WinSparkle gets an error while it is
    ///   attempting to check for updates.
    /// </summary>
    {$ENDREGION}
    property OnError:TProc read fOnError write SetOnError;
    {$REGION 'This event is called when the updater closes and the install file is launched.'}
    /// <summary>
    ///   This event is called when the updater closes and the install file is
    ///   launched.
    /// </summary>
    {$ENDREGION}
    property OnShutDown:TProc read fOnShutDown write SetOnShutDown;
    {$REGION 'This event is triggered when an update is found.'}
    /// <summary>
    ///   This event is triggered when an update is found.
    /// </summary>
    {$ENDREGION}
    property OnDidFindUpdate: TProc read fOnDidFindUpdate write SetOnDidFindUpdate;
    {$REGION 'This event is triggered when an update is not found.'}
    /// <summary>
    ///   This event is triggered when an update is not found.
    /// </summary>
    {$ENDREGION}
    property OnDidNotFindUpdate: TProc read fOnDidNotFindUpdate
      write SetOnDidNotFindUpdate;
    {$REGION 'This event is raised when the user closes the window or does not install the update ("Skip/Remind Later").'}
    /// <summary>
    ///   This event is raised when the user closes the window or does not
    ///   install the update ("Skip/Remind Later").
    /// </summary>
    {$ENDREGION}
    property OnUpdateCancelled: TProc read fOnUpdateCancelled
      write SetOnUpdateCancelled;
    property OnCanShutDown: TFunc<Boolean> read fOnCanShutDown
      write SetOnCanShutDown;
  end;


implementation

uses
	Winapi.Windows, FMX.Dialogs, System.DateUtils;

////
/// This variable and the DoProcedure(s) are used to trigger the
/// correct events in the class. They need to be declared outside
/// the class to work properly with the imported DLL methods
///
var
  callbackProcs: TCallbackArray;
  callbackFuncs: TCallbackArrayFunctions;

procedure DoOnError; cdecl;
var
  tmpProc: TProc;
begin
  if Assigned(callbackProcs[cbError]) then
  begin
    tmpProc:=callbackProcs[cbError];
    tmpProc;
  end;
end;

procedure DoOnUpdateCancelled; cdecl;
var
  tmpProc: TProc;
begin
  if Assigned(callbackProcs[cbUpdateCancelled]) then
  begin
    tmpProc:=callbackProcs[cbUpdateCancelled];
    tmpProc;
  end;
end;

procedure DoOnShutDown; cdecl;
var
  tmpProc: TProc;
begin
  if Assigned(callbackProcs[cbShutdown]) then
  begin
    tmpProc:=callbackProcs[cbShutdown];
    tmpProc;
  end;
end;

procedure DoOnUpdateFound; cdecl;
var
  tmpProc: TProc;
begin
  if Assigned(callbackProcs[cbDidFindUpdate]) then
  begin
    tmpProc:=callbackProcs[cbDidFindUpdate];
    tmpProc;
  end;
end;

procedure DoOnUpdateNotFound; cdecl;
var
  tmpProc: TProc;
begin
  if Assigned(callbackProcs[cbDidNotFindUpdate]) then
  begin
    tmpProc:=callbackProcs[cbDidNotFindUpdate];
    tmpProc;
  end;
end;

function DoOnCanShutdown:boolean; cdecl;
var
  tmpFunc: TFunc<Boolean>;
begin
  if Assigned(callbackFuncs[cbCanShutdown]) then
  begin
    tmpFunc:=callbackFuncs[cbCanShutdown];
    result:=tmpFunc;
  end;

end;

/////////

{ TWinSparkle }

constructor TWinSparkle.Create;
begin
  Create('');
end;

procedure TWinSparkle.CheckUpdate(const updateType: TUpdateType);
begin
  case updateType of
    utWithoutUI: CheckUpdateWithoutUI;
    utWithUI: CheckUpdateWithUI;
    utWithUIAndInstall: CheckUpdateWithUIandInstall;
  end;
end;

procedure TWinSparkle.CheckUpdateWithoutUI;
begin
  ExecuteSimpleProcedure('win_sparkle_check_update_without_ui');
end;

procedure TWinSparkle.CheckUpdateWithUI;
begin
  ExecuteSimpleProcedure('win_sparkle_check_update_with_ui');
end;

procedure TWinSparkle.CheckUpdateWithUIandInstall;
begin
  ExecuteSimpleProcedure('win_sparkle_check_update_with_ui_and_install');
end;

procedure TWinSparkle.Cleanup;
begin
  executeSimpleProcedure('win_sparkle_cleanup');
end;

constructor TWinSparkle.Create(const newPath: string);
begin
  fDLLPath:=newPath;
  fHandle:=SafeLoadLibrary(fDLLPath+WinSparkleLib);
end;

destructor TWinSparkle.Destroy;
begin
  Cleanup;
  FreeLibrary(fHandle);
  inherited;
end;

procedure TWinSparkle.ExecuteReferencedFunctionBoolean(
  const funcName: PWideChar; const funcType: TCallbackTypeFunc;
  const newFunc: TFunc<Boolean>);
type
  TNewFuncCallback = function: boolean; cdecl;
  TNewProc = procedure (newCallBack: TNewFuncCallback); cdecl;

var
  tmpProc: TNewProc;

begin
  if not getDLLLoaded then
    Exit;
  if not Assigned(newFunc) then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,funcName);
  if Assigned(tmpProc) then
  begin
    case funcType of
      cbCanShutdown: begin
                       callbackFuncs[cbCanShutdown]:=newFunc;
                       tmpProc(DoOnCanShutdown);
                      end;
    end;
  end;
end;

procedure TWinSparkle.ExecuteReferencedProcedure(const procName: PWideChar;
  const procType: TCallbackType; const newProc: TProc);

type
  TNewProcCallback = procedure; cdecl;
  TNewProc = procedure (newCallBack: TNewProcCallback); cdecl;

var
  tmpProc: TNewProc;

begin
  if not getDLLLoaded then
    Exit;
  if not Assigned(newProc) then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,procName);
  if Assigned(tmpProc) then
  begin
    case procType of
      cbError: begin
                 callbackProcs[cbError]:=newProc;
                 tmpProc(DoOnError);
               end;
      cbShutdown: begin
                    callbackProcs[cbShutdown]:=newProc;
                    tmpProc(DoOnShutDown);
                  end;
      cbDidFindUpdate: begin
                         callbackProcs[cbDidFindUpdate]:=newProc;
                         tmpProc(DoOnUpdateFound);
                       end;
      cbDidNotFindUpdate: begin
                            callbackProcs[cbDidNotFindUpdate]:=newProc;
                            tmpProc(DoOnUpdateNotFound);
                          end;
      cbUpdateCancelled: begin
                           callbackProcs[cbUpdateCancelled]:=newProc;
                           tmpProc(DoOnUpdateCancelled);
                         end;

    end;
  end;
end;

procedure TWinSparkle.ExecuteSimpleProcedure(const procName: PWideChar);
var
  tmpProc: procedure; cdecl;
begin
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,procName);
  if Assigned(tmpProc) then
    tmpProc;
end;

function TWinSparkle.GetAppBuildVersion: string;
begin
  result:=fAppBuildVersion;
end;

function TWinSparkle.GetAppcastURL: string;
begin
  result:=fAppcastURL;
end;

function TWinSparkle.GetAutoCheckForUpdates: Boolean;
var
  tmpFunc: function: Boolean; cdecl;
begin
  result:=False;
  if not getDLLLoaded then
    Exit;
  @tmpFunc:=GetProcAddress(fHandle,'win_sparkle_get_automatic_check_for_updates');
  if Assigned(tmpFunc) then
    result:=Boolean(tmpFunc)
  else
    result:=false;
end;

function TWinSparkle.getDLLLoaded: Boolean;
begin
  result:=fHandle <> 0;
end;


function TWinSparkle.GetLastCheckTime: TDateTime;
var
  tmpFunc: function: LongInt; cdecl;
begin
  result:=Now;
  if not getDLLLoaded then
    Exit;
  @tmpFunc:=GetProcAddress(fHandle,'win_sparkle_get_last_check_time');
  if Assigned(tmpFunc) then
    result:=UnixToDateTime(tmpFunc)
  else
    result:=Now;
end;

function TWinSparkle.GetRegistryPath: string;
begin
  if fRegistryPath='' then
    result:='Software\'+fCompanyName+'\'+fAppName+'\WinSparkle'
  else
    result:=fRegistryPath;
end;

function TWinSparkle.GetUpdateCheckInterval: Integer;
var
  tmpFunc: function: integer; cdecl;
begin
  if not getDLLLoaded then
    Exit;
  @tmpFunc:=GetProcAddress(fHandle,'win_sparkle_get_update_check_interval');
  if Assigned(tmpFunc) then
    result:=integer(tmpFunc);
end;

procedure TWinSparkle.Init;
begin
  ExecuteSimpleProcedure('win_sparkle_init');
end;

procedure TWinSparkle.ResetUpdateCheckInterval;
begin
  Cleanup;
  init;
end;

procedure TWinSparkle.SetAppBuildVersion(const newAppBuildVersion: string);
var
  tmpProc: procedure (appBuild: PWideChar); cdecl;
begin
  fAppBuildVersion:=newAppBuildVersion;
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_app_build_version');
  if Assigned(tmpProc) then
    tmpProc(PWideChar(newAppBuildVersion));
end;

procedure TWinSparkle.SetAppcastURL(const newURL: string);
var
  tmpProc: procedure (URL: PAnsiChar); cdecl;
begin
  fAppcastURL:=newURL;
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_appcast_url');
  if Assigned(tmpProc) then
    tmpProc(PAnsiChar(AnsiString(newURL)));
end;

procedure TWinSparkle.SetAppDetails(const companyName, appName,
  appVersion: string);
var
  tmpProc: procedure (cName, aName, aVersion: PWideChar); cdecl;
begin
  fCompanyName:=companyName;
  fAppName:=appName;
  fAppVersion:=appVersion;
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_app_details');
  if Assigned(tmpProc) then
    tmpProc(PWideChar(companyName),
      PWideChar(appName), PWideChar(appVersion));
end;

procedure TWinSparkle.SetAutoCheckForUpdates(const autoCheck: Boolean);
var
  tmpProc: procedure (state: Integer); cdecl;
begin
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_automatic_check_for_updates');
  if Assigned(tmpProc) then
    tmpProc(Integer(autoCheck));
end;


procedure TWinSparkle.SetLang(const newLang: string);
var
  tmpProc: procedure (newL: PAnsiChar); cdecl;
begin
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_lang');
  if Assigned(tmpProc) then
    tmpProc(PAnsiChar(AnsiString(newLang)));
end;

procedure TWinSparkle.SetLangID(const newLandID: ShortInt);
var
  tmpProc: procedure (newLID: ShortInt); cdecl;
begin
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_langid');
  if Assigned(tmpProc) then
    tmpProc(newLandID);
end;

procedure TWinSparkle.SetOnCanShutDown(const newFunc: TFunc<Boolean>);
begin
  fOnCanShutDown:=newFunc;
  ExecuteReferencedFunctionBoolean('win_sparkle_set_can_shutdown_callback',
    cbCanShutdown, newFunc);
end;

procedure TWinSparkle.SetOnDidFindUpdate(const newProc: TProc);
begin
  fOnDidFindUpdate:=newProc;
  ExecuteReferencedProcedure('win_sparkle_set_did_find_update_callback',
    cbDidFindUpdate, newProc);
end;

procedure TWinSparkle.SetOnDidNotFindUpdate(const newProc: TProc);
begin
  fOnDidNotFindUpdate:=newProc;
  ExecuteReferencedProcedure('win_sparkle_set_did_not_find_update_callback',
    cbDidNotFindUpdate, newProc);
end;

procedure TWinSparkle.SetOnError(const newProc: TProc);
begin
 fOnError:=newProc;
 ExecuteReferencedProcedure('win_sparkle_set_error_callback',
  cbError, newProc);
end;

procedure TWinSparkle.SetOnShutDown(const newProc: TProc);
begin
 fOnShutDown:=newProc;
 ExecuteReferencedProcedure('win_sparkle_set_shutdown_request_callback',
  cbShutdown, newProc);
end;

procedure TWinSparkle.SetOnUpdateCancelled(const newProc: TProc);
begin
  fOnUpdateCancelled:=newProc;
  ExecuteReferencedProcedure('win_sparkle_set_update_cancelled_callback',
    cbUpdateCancelled, newProc);
end;

procedure TWinSparkle.SetRegistryPath(const newPath: string);
var
  tmpProc: procedure (nPath: PAnsiChar); cdecl;
begin
  fRegistryPath:=newPath;
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_registry_path');
  if Assigned(tmpProc) then
    tmpProc(PAnsiChar(AnsiString(newPath)));
end;

procedure TWinSparkle.SetUpdateCheckInterval(const newInterval: Integer);
var
  tmpProc: procedure (interval: Integer); cdecl;
begin
  if not getDLLLoaded then
    Exit;
  @tmpProc:=GetProcAddress(fHandle,'win_sparkle_set_update_check_interval');
  if Assigned(tmpProc) then
    tmpProc(newInterval);
end;

end.
