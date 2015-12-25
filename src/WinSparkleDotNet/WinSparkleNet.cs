using System;
using System.Reflection;

namespace WinSparkleDotNet
{
    /// <summary>
    ///     WinSparkle native C# API.
    /// </summary>
    public sealed class WinSparkleNet
    {
        /// <summary>
        ///     Gets AssemblyName.Name of assembly.
        /// </summary>
        /// <seealso cref="System.Reflection.AssemblyName.Name"/>
        public string Name { get; private set; }
        /// <summary>
        ///     Gets AssemblyCompanyAttribute.Company of assembly.
        /// </summary>
        /// <seealso cref="System.Reflection.AssemblyCompanyAttribute"/>
        public string Company { get; private set; }
        private string _version;
        /// <summary>
        ///     Gets or sets the major, minor, build, and revision numbers of the assembly.
        /// </summary>
        /// <seealso cref="System.Reflection.AssemblyName.Version"/>
        public string Version
        {
            get { return _version; }
            set
            {
                if (_initialized) return;
                _version = value;
                WinSparkle.SetAppBuildVersion(value);
            }
        }
        private string _appCastUrl;
        /// <summary>
        ///     Gets AppCastUrlAttribute.AppCastUrl of assembly.
        /// </summary>
        /// <seealso cref="WinSparkleDotNet.AppCastUrlAttribute"/>
        public string AppCastUrl
        {
            get { return _appCastUrl; }
            set
            {
                if (_initialized) return;
                _appCastUrl = value;
                WinSparkle.SetAppcastUrl(value);
            }
        }
        /// <summary>
        ///     Gets assembly of the process executable in the default application domain.
        /// </summary>
        /// <seealso cref="System.Reflection.Assembly"/>
        public Assembly Assembly { get; private set; }

        private bool _initialized = false;

        /// <summary>
        ///     Load data from (Entry) Assembly.
        /// </summary>
        /// <see cref="AppCastUrl"/>
        /// <see cref="Version"/>
        /// <see cref="Initialize"/>
        public WinSparkleNet()
        {
            try
            {
                Assembly = Assembly.GetEntryAssembly();
                Name = Assembly.GetName().Name;
                Company = ((AssemblyCompanyAttribute)Assembly.GetCustomAttributes(typeof(AssemblyCompanyAttribute), false)[0]).Company;
                Version = Assembly.GetName().Version.ToString();
                AppCastUrl = ((AppCastUrlAttribute)Assembly.GetCustomAttributes(typeof(AppCastUrlAttribute), false)[0]).AppCastUrl;
            }
            catch (Exception ex)
            {
                throw ex;
            }            
        }

        /// <summary>
        ///     Starts WinSparkle, with data get from Assembly.
        /// </summary>
        /// <see cref="Cleanup"/>
        public void Initialize()
        {
            WinSparkle.SetAppcastUrl(AppCastUrl);
            WinSparkle.SetAppDetails(Company, Name, Version);
            WinSparkle.Initialize();
            _initialized = true;
        }

        /// <summary>
        ///     Checks if an update is available.
        /// </summary>
        /// <param name="withUi"> With progress UI (default : yes)</param>
        /// <param name="install"> Installing the update if one is available (default : false) </param>
        public void CheckForUpdate(bool withUi = true, bool install = false)
        {
            if (withUi)
            {
                if(!install)
                    WinSparkle.CheckUpdateWithUI();
                else
                    WinSparkle.CheckUpdateWithUIandInstall();
            }
            else
            {
                WinSparkle.CheckUpdateWithoutUI();
            }
        }

        /// <summary>
        ///     Cleans up after WinSparkle.
        ///     <seealso cref="WinSparkle.Cleanup"/>
        /// </summary>
        public void Cleanup()
        {
            WinSparkle.Cleanup();
        }

        /// <summary>
        ///     Sets UI language from its ISO code.
        ///     <seealso cref="WinSparkle.SetLanguage"/>
        /// </summary>
        /// <param name="language"> ISO 639 language code with an optional ISO 3116 country
        ///                       code
        ///  </param>
        public void SetLanguage(string language)
        {
            if (!_initialized)
                WinSparkle.SetLanguage(language);
        }

        /// <summary>
        ///     Set the registry path where settings will be stored.
        ///     <seealso cref="WinSparkle.SetRegistryPath"/>
        /// </summary>
        /// <param name="path"></param>
        public void SetRegistryPath(string path)
        {
            if (!_initialized)
                WinSparkle.SetRegistryPath(path);
        }

        /// <summary>
        ///     Get the last check time.
        ///     <seealso cref="WinSparkle.LastCheckTime"/>
        /// </summary>
        public DateTime LastCheckTime
        {
            get { return WinSparkle.LastCheckTime; }
        }

        /// <summary>  
        ///     Get or set the update interval. 
        ///     The minimum update interval is 3600 seconds (1 hour). 
        ///     <seealso cref="WinSparkle.UpdateInterval"/>
        /// </summary>
        public TimeSpan UpdateInterval
        {
            get { return WinSparkle.UpdateInterval; }
            set
            {
                if (!_initialized)
                    WinSparkle.UpdateInterval = value;
            }
        }

        /// <summary>
        ///     Get or set a value indicating whether the automatic check for updates.
        ///     <seealso cref="WinSparkle.AutomaticCheckForUpdates"/>
        /// </summary>
        public bool AutomaticCheckForUpdates
        {
            get { return WinSparkle.AutomaticCheckForUpdates; }
            set
            {
                if (!_initialized)
                    WinSparkle.AutomaticCheckForUpdates = value;
            }
        }

        #region Callback
        /// <summary>
        ///     Set callback to be called when the updater encounters an error.
        ///     <seealso cref="WinSparkle.SetErrorCallback"/>
        /// </summary>
        /// <param name="callback"></param>
        public void SetErrorCallback(WinSparkle.ErrorCallback callback)
        {
            if(!_initialized)
                WinSparkle.SetErrorCallback(callback);
        }

        /// <summary>
        ///     Set callback for querying the application if it can be closed.
        ///     <seealso cref="WinSparkle.SetCanShutdownCallback"/>
        /// </summary>
        /// <param name="callback"></param>
        public void SetCanShutdownCallback(WinSparkle.CanShutdownCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetCanShutdownCallback(callback);
        }

        /// <summary>
        ///     Set callback for shutting down the application.
        ///     <seealso cref="WinSparkle.SetShutdownRequestCallback"/>
        /// </summary>
        /// <param name="callback"></param>
        public void SetShutdownRequestCallback(WinSparkle.ShutdownRequestCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetShutdownRequestCallback(callback);
        }

        /// <summary>
        ///     Set callback to be called when the updater did find an update.
        ///     <seealso cref="WinSparkle.SetDidFindUpdateCallback"/>
        /// </summary>
        /// <param name="callback"></param>
        public void SetDidFindUpdateCallback(WinSparkle.DidFindUpdateCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetDidFindUpdateCallback(callback);
        }

        /// <summary>
        ///     Set callback to be called when the updater did not find an update.
        ///     <seealso cref="WinSparkle.SetDidNotFindUpdateCallback"/>
        /// </summary>
        /// <param name="callback"></param>
        public void SetDidNotFindUpdateCallback(WinSparkle.DidNotFindUpdateCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetDidNotFindUpdateCallback(callback);
        }

        /// <summary>
        ///     Set callback to be called when the user cancels a download.
        ///     <seealso cref="WinSparkle.SetUpdateCancelledCallback"/>
        /// </summary>
        /// <param name="callback"></param>
        public void SetUpdateCancelledCallback(WinSparkle.UpdateCancelledCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetUpdateCancelledCallback(callback);
        }
        #endregion

    }
}
