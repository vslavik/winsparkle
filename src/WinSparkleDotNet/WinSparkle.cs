using System;
using System.Runtime.InteropServices;

namespace WinSparkleDotNet
{
    /// <summary>
    ///     WinSparkle DLL Wrapper
    ///     Is a 1-to-1 mapping of the WinSparkle C API with some adjustment.
    ///     Don't use directly unless you know what you do !
    ///     Otherwise use <see cref="WinSparkleNet"/> API. 
    /// </summary>
    /// <see cref="WinSparkleNet"/>
    public sealed class WinSparkle
    {
        /// <summary>   Get the last check time. </summary>
        ///
        /// <value> The last check time. </value>
        /// 
        /// <remarks> Can only be called @em before the first call to <see cref="Initialize"/> </remarks>
        public static DateTime LastCheckTime
        {
            get
            {
                int time = GetLastCheckTime();
                if (time > 0)
                    return new System.DateTime(1970, 1, 1).AddSeconds(time);
                else
                    return System.DateTime.MinValue;
            }
        }

        /// <summary>   Get or set the update interval. 
        ///             The minimum update interval is 3600 seconds (1 hour). 
        /// </summary>
        ///
        /// <value> The update interval. </value>
        /// 
        /// <remarks> Can only be called @em before the first call to <see cref="Initialize"/> </remarks>
        public static TimeSpan UpdateInterval
        {
            get
            {
                return TimeSpan.FromSeconds(GetUpdateCheckInterval());
            }
            set
            {
                SetUpdateCheckInterval((int)value.TotalSeconds);
            }
        }

        /// <summary>
        ///     Get or set a value indicating whether the automatic check for updates.
        /// </summary>
        ///
        /// <value> true if automatic check for updates, false if not. </value>
        /// 
        /// <remarks> Can only be called @em before the first call to <see cref="Initialize"/> </remarks>
        public static bool AutomaticCheckForUpdates
        {
            get
            {
                return GetAutomaticCheckForUpdates() != 0;
            }
            set
            {
                SetAutomaticCheckForUpdates(value ? 1 : 0);
            }
        }

#region DLLWrapping
        /// <summary>   
        ///     <para>
        ///         Starts WinSparkle.
        ///     </para>
        ///     <para>
        ///         If WinSparkle is configured to check for updates on startup, proceeds
        ///         to perform the check. You should only call this function when your app
        ///         is initialized and shows its main window.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>   This call doesn't block and returns almost immediately. If an
        ///          update is available, the respective UI is shown later from a separate
        ///          thread. </remarks>
        /// 
        /// <see cref="Cleanup"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_init", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Initialize();

        /// <summary>  Cleans up after WinSparkle.
        ///    Should be called by the app when it's shutting down. Cancels any
        ///    pending Sparkle operations and shuts down its helper threads. </summary>
        ///
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_cleanup", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Cleanup();

        /// <summary>   
        ///     <para>
        ///          Sets UI language from its ISO code.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>  This function must be called before <see cref="Initialize"/> </remarks>
        ///
        /// <param name="lang">   ISO 639 language code with an optional ISO 3116 country
        ///                       code, e.g. "fr", "pt-PT", "pt-BR" or "pt_BR"
        ///  </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_lang", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetLanguage(
            [param: MarshalAs(UnmanagedType.AnsiBStr)] string lang
            );

        /// <summary>   
        ///     <para>
        ///         Sets URL for the app's appcast.
        ///     </para>
        ///     <para>
        ///         If this function isn't called by the app, the URL is obtained from
        ///         Windows resource named "FeedURL" of type "APPCAST".
        ///     </para>
        /// </summary>
        ///
        /// <remarks>  Can only be called @em before the first call to <see cref="Initialize"/> </remarks>
        ///
        /// <param name="url">   URL of the appcast. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_appcast_url", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetAppcastUrl(
            [param: MarshalAs(UnmanagedType.AnsiBStr)] string url
            );

        /// <summary>   
        ///     <para>
        ///         Sets application metadata.
        ///     </para>
        ///     <para>
        ///         Normally, these are taken from VERSIONINFO/StringFileInfo resources,
        ///         but if your application doesn't use them for some reason, using this
        ///         function is an alternative.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         <paramref name="companyName"/> and <paramref name="appName"/> are used to determine
        ///         the location of WinSparkle settings in registry. 
        ///     </para>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <param name="companyName">   Company name of the vendor. </param>
        /// <param name="appName">   Application name. This is both shown to the user
        ///                         and used in HTTP User-Agent header. </param>
        /// <param name="appVersion">   Version of the app, as string (e.g. "1.2" or "1.2rc1"). </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_app_details", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetAppDetails(
            [param: MarshalAs(UnmanagedType.LPWStr)] string companyName,
            [param: MarshalAs(UnmanagedType.LPWStr)] string appName,
            [param: MarshalAs(UnmanagedType.LPWStr)] string appVersion
            );

        /// <summary>   
        ///     <para>
        ///         Sets application build version number.
        ///     </para>
        ///     <para>
        ///         This is the internal version number that is not normally shown to the user.
        ///         It can be used for finer granularity that official release versions, e.g. for
        ///         interim builds.
        ///     </para>
        ///     <para>
        ///         If this function is called, then the provided *build* number is used for comparing
        ///         versions; it is compared to the "version" attribute in the appcast and corresponds
        ///         to OS X Sparkle's CFBundleVersion handling. If used, then the appcast must
        ///         also contain the "shortVersionString" attribute with human-readable display
        ///         version string. The version passed to win_sparkle_set_app_details()
        ///         corresponds to this and is used for display.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <param name="buildVersion">   The version number. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_app_build_version", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetAppBuildVersion(
            [param: MarshalAs(UnmanagedType.LPWStr)] string buildVersion
            );

        /// <summary>   
        ///     <para>
        ///         Set the registry path where settings will be stored.
        ///     </para>
        ///     <para>
        ///         Normally, these are stored in "HKCU\Software\&lt;company_name>\&lt;app_name>\WinSparkle"
        ///         but if your application needs to store the data elsewhere for
        ///         some reason, using this function is an alternative.
        ///     </para>
        ///     <para>
        ///         Note that @a path is relative to HKCU/HKLM root and the root is not part of it.
        ///     </para>
        /// </summary>
        /// 
        /// <example>
        ///     <code>SetRegistryPath(@"Software\My App\Updates");</code>
        /// </example>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <param name="path">   Registry path where settings will be stored. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_registry_path", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetRegistryPath(
            [param: MarshalAs(UnmanagedType.LPWStr)] string path
            );

        /// <summary>   
        ///     <para>
        ///         Sets whether updates are checked automatically or only through a manual call.
        ///     </para>
        ///     <para>
        ///         If disabled, win_sparkle_check_update_with_ui() must be used explicitly.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <param name="enableAutomaticUpdates">   1 to have updates checked automatically, 0 otherwise. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_automatic_check_for_updates", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetAutomaticCheckForUpdates(
            [param: MarshalAs(UnmanagedType.I4)] int enableAutomaticUpdates
            );

        /// <summary>   
        ///     <para>
        ///         Gets the automatic update checking state
        ///     </para>
        ///     <para>
        ///         Defaults to 0 when not yet configured (as happens on first start).
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <returns> 1 if updates are set to be checked automatically, 0 otherwise </returns>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_get_automatic_check_for_updates", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        private static extern int GetAutomaticCheckForUpdates();

        /// <summary>   
        ///     <para>
        ///         Sets the automatic update interval.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <param name="intervalSeconds">   The interval in seconds between checks for updates.
        ///                     The minimum update interval is 3600 seconds (1 hour). </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_update_check_interval", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetUpdateCheckInterval(
            [param: MarshalAs(UnmanagedType.I4)] int intervalSeconds
            );

        /// <summary>   
        ///     <para>
        ///         Gets the automatic update interval in seconds.
        ///     </para>
        ///     <para>
        ///         Default value is one day.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <returns>   The update check interval in seconds. </returns>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_get_update_check_interval", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        private static extern int GetUpdateCheckInterval();

        /// <summary>   
        ///     <para>
        ///         Gets the time for the last update check.
        ///     </para>
        ///     <para>
        ///         Default value is -1, indicating that the update check has never run.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        /// </remarks>
        ///
        /// <returns>   The last check time internal. </returns>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_get_last_check_time", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I4)]
        private static extern int GetLastCheckTime();

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ErrorCallback();

        /// <summary>   
        ///     <para>
        ///         Set callback to be called when the updater encounters an error.
        ///     </para>
        /// </summary>
        /// <param name="callback"> The callback. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_error_callback", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetErrorCallback(
            [param: MarshalAs(UnmanagedType.FunctionPtr)] ErrorCallback callback
            );

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate bool CanShutdownCallback();

        /// <summary>   
        ///     <para>
        ///         Set callback for querying the application if it can be closed.
        ///     </para>
        ///     <para>
        ///         This callback will be called to ask the host if it's ready to shut down,
        ///         before attempting to launch the installer. The callback returns TRUE if
        ///         the host application can be safely shut down or FALSE if not (e.g. because
        ///         the user has unsaved documents).
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        ///     <para>
        ///         There's no guarantee about the thread from which the callback is called,
        ///         except that it certainly *won't* be called from the app's main thread.
        ///         Make sure the callback is thread-safe.
        ///     </para>
        /// </remarks>
        ///
        /// <param name="callback"> The callback. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_can_shutdown_callback", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetCanShutdownCallback(
            [param: MarshalAs(UnmanagedType.FunctionPtr)] CanShutdownCallback callback
            );

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ShutdownRequestCallback();

        /// <summary>   
        ///     <para>
        ///         Set callback for shutting down the application.
        ///     </para>
        ///     <para>
        ///         This callback will be called to ask the host to shut down immediately after
        ///         launching the installer. Its implementation should gracefully terminate the
        ///         application.
        ///     </para>
        ///     <para>
        ///         It will only be called if the call to the callback set with
        ///         win_sparkle_set_can_shutdown_callback() returns TRUE.
        ///     </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Can only be called @em before the first call to <see cref="Initialize"/>
        ///     </para>
        ///     <para>
        ///         There's no guarantee about the thread from which the callback is called,
        ///         except that it certainly *won't* be called from the app's main thread.
        ///         Make sure the callback is thread-safe.
        ///     </para>
        /// </remarks>
        ///
        /// <param name="callback"> The callback. </param>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_shutdown_request_callback", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetShutdownRequestCallback(
            [param:MarshalAs(UnmanagedType.FunctionPtr)] ShutdownRequestCallback callback
            );

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void DidFindUpdateCallback();

        /// <summary>   
        ///     <para>
        ///         Set callback to be called when the updater did find an update.
        ///     </para>
        ///     <para>
        ///         This is useful in combination with
        ///         <see cref="CheckUpdateWithUIandInstall"/> as it allows you to perform
        ///         some action after WinSparkle checks for updates.
        ///     </para>
        /// </summary>
        /// <param name="callback"> The callback. </param>
        /// 
        /// <see cref="DidNotFindUpdateCallback"/>
        /// <see cref="CheckUpdateWithUIandInstall"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_did_find_update_callback", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetDidFindUpdateCallback(
            [param: MarshalAs(UnmanagedType.FunctionPtr)] DidFindUpdateCallback callback
            );

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void DidNotFindUpdateCallback();

        /// <summary>   
        ///     <para>
        ///         Set callback to be called when the updater did not find an update.
        ///     </para>
        ///     <para>
        ///         This is useful in combination with
        ///         <see cref="CheckUpdateWithUIandInstall"/> as it allows you to perform
        ///         some action after WinSparkle checks for updates.
        ///     </para>
        /// </summary>
        /// <param name="callback"> The callback. </param>
        /// 
        /// <see cref="DidFindUpdateCallback"/>
        /// <see cref="CheckUpdateWithUIandInstall"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_did_not_find_update_callback", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetDidNotFindUpdateCallback(
            [param: MarshalAs(UnmanagedType.FunctionPtr)] DidNotFindUpdateCallback callback
            );

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void UpdateCancelledCallback();

        /// <summary>   
        ///     <para>
        ///         Set callback to be called when the user cancels a download.
        ///     </para>
        ///     <para>
        ///         This is useful in combination with
        ///         <see cref="CheckUpdateWithUIandInstall"/> as it allows you to perform
        ///         some action when the installation is interrupted.
        ///     </para>
        /// </summary>
        /// <param name="callback"> The callback. </param>
        /// 
        /// <see cref="CheckUpdateWithUIandInstall"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_set_update_cancelled_callback", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetUpdateCancelledCallback(
            [param: MarshalAs(UnmanagedType.FunctionPtr)] UpdateCancelledCallback callback
            );

        /// <summary>   
        ///     <para>
        ///         Checks if an update is available, showing progress UI to the user.
        ///     </para>
        ///     <para>
        ///         Normally, WinSparkle checks for updates on startup and only shows its UI
        ///         when it finds an update. If the application disables this behavior, it
        ///         can hook this function to "Check for updates..." menu item..
        ///     </para>
        ///     <para>
        ///         When called, background thread is started to check for updates. A small
        ///         window is shown to let the user know the progress. If no update is found,
        ///         the user is told so. If there is an update, the usual "update available"
        ///         window is shown.
        ///     </para>
        ///     <para>
        ///         This function returns immediately.  
        ///      </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         Because this function is intended for manual, user-initiated checks
        ///         for updates, it ignores "Skip this version" even if the user checked
        ///         it previously.
        ///     </para>
        /// </remarks>
        /// 
        /// <see cref="CheckUpdateWithoutUI"/>
        /// <see cref="CheckUpdateWithUIandInstall"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_check_update_with_ui", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CheckUpdateWithUI();

        /// <summary>   
        ///     <para>
        ///         Checks if an update is available, showing progress UI to the user and
        ///         immediately installing the update if one is available.
        ///     </para>
        ///     <para>
        ///         This is useful for the case when users should almost always use the
        ///         newest version of your software.When called, WinSparkle will check for
        ///         updates showing a progress UI to the user. If an update is found the update
        ///         prompt will be skipped and the update will be installed immediately.
        ///     </para>
        ///     
        /// </summary>
        /// <remarks>
        ///     <para>
        ///         If your application expects to do something after checking for updates you
        ///         may wish to use win_sparkle_set_did_not_find_update_callback() and
        ///         win_sparkle_set_update_cancelled_callback().
        ///     </para>
        /// </remarks>
        /// <see cref="CheckUpdateWithUI"/>
        /// <see cref="CheckUpdateWithoutUI"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_check_update_with_ui_and_install", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CheckUpdateWithUIandInstall();

        /// <summary>   
        ///     <para>
        ///         Checks if an update is available.
        ///     </para>
        ///     <para>
        ///         No progress UI is shown to the user when checking. If an update is
        ///         available, the usual "update available" window is shown; this function
        ///         is *not* completely UI-less.
        ///     </para>
        ///     <para>
        ///         Use with caution, it usually makes more sense to use the automatic update
        ///         checks on interval option or manual check with visible UI.
        ///     </para>
        ///     <para>
        ///         This function returns immediately.  
        ///      </para>
        /// </summary>
        ///
        /// <remarks>
        ///     <para>
        ///         This function respects "Skip this version" choice by the user.
        ///     </para>
        /// </remarks>
        /// 
        /// <see cref="CheckUpdateWithUI"/>
        [DllImport("WinSparkle.dll", EntryPoint = "win_sparkle_check_update_without_ui", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CheckUpdateWithoutUI();
#endregion
    }
}
