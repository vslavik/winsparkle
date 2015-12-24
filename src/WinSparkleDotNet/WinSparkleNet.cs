using System;
using System.Reflection;

namespace WinSparkleDotNet
{
    public sealed class WinSparkleNet
    {
        public string Name { get; private set; }
        public string Company { get; private set; }
        private string _version;
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
        ///     Starts WinSparkle, with data get from Assembly infos.
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
        /// </summary>
        public void Cleanup()
        {
            WinSparkle.Cleanup();
        }

        /// <summary>
        ///     Sets UI language from its ISO code.
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
        /// </summary>
        /// <param name="path"></param>
        public void SetRegistryPath(string path)
        {
            if (!_initialized)
                WinSparkle.SetRegistryPath(path);
        }

        /// <summary>
        ///     Get the last check time.
        /// </summary>
        public DateTime LastCheckTime
        {
            get { return WinSparkle.LastCheckTime; }
        }

        /// <summary>   Get or set the update interval. 
        ///             The minimum update interval is 3600 seconds (1 hour). 
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

        public void SetErrorCallback(WinSparkle.ErrorCallback callback)
        {
            if(!_initialized)
                WinSparkle.SetErrorCallback(callback);
        }

        public void SetCanShutdownCallback(WinSparkle.CanShutdownCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetCanShutdownCallback(callback);
        }

        public void SetShutdownRequestCallback(WinSparkle.ShutdownRequestCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetShutdownRequestCallback(callback);
        }

        public void SetDidFindUpdateCallback(WinSparkle.DidFindUpdateCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetDidFindUpdateCallback(callback);
        }

        public void SetDidNotFindUpdateCallback(WinSparkle.DidNotFindUpdateCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetDidNotFindUpdateCallback(callback);
        }

        public void SetUpdateCancelledCallback(WinSparkle.UpdateCancelledCallback callback)
        {
            if (!_initialized)
                WinSparkle.SetUpdateCancelledCallback(callback);
        }
        #endregion

    }
}
