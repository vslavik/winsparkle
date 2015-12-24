using System;

namespace WinSparkleDotNet
{
    [AttributeUsage(AttributeTargets.Assembly)]
    public sealed class AppCastUrlAttribute : Attribute
    {
        private string _appCastURL;

        public AppCastUrlAttribute(string appCastUrl)
        {
            _appCastURL = appCastUrl;
        }

        public string AppCastUrl
        {
            get { return _appCastURL; }
        }
    }
}
