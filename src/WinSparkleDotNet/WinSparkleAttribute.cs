using System;

namespace WinSparkleDotNet
{
    /// <summary>
    ///     WinSparkleNet custom assembly attribut for storing App Cast URL.    
    /// </summary>
    /// <example>
    ///     <c>[assembly: AppCastUrl("http://winsparkle.org/example/appcast.xml")]</c>
    /// </example>
    /// <see cref="System.Reflection.Assembly"/>
    /// <see cref="System.Attribute"/>
    [AttributeUsage(AttributeTargets.Assembly)]
    public sealed class AppCastUrlAttribute : Attribute
    {
        private string _appCastURL;

        public AppCastUrlAttribute(string appCastUrl)
        {
            _appCastURL = appCastUrl;
        }

        /// <summary>
        ///     Gets the App Cast URL from the assembly.
        /// </summary>
        public string AppCastUrl
        {
            get { return _appCastURL; }
        }
    }
}
