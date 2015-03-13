using System;
using System.Reflection;
using System.Threading;
using System.Windows;
using WinSparkleDotNet;

namespace WinSparkleDotNetExample
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private SynchronizationContext _context;

        public string Version { get { return "Current version: " + Assembly.GetExecutingAssembly().GetName().Version; } }

        public MainWindow()
        {
            _context = SynchronizationContext.Current;
            Closed += OnClosed;

            InitializeComponent();

            try
            {
                // Set URL to retrieve the appcast RSS feed
                WinSparkle.SetAppcastUrl("http://localhost:8000/xml/appcast.xml");

                // Provide app details (Winsparkle doesn't infer this information correctly from a WPF application)
                var assembly = Assembly.GetExecutingAssembly();
                var company = (AssemblyCompanyAttribute)assembly.GetCustomAttributes(typeof(AssemblyCompanyAttribute), false)[0];
                var version = assembly.GetName().Version;
                WinSparkle.SetAppDetails(company.Company, assembly.GetName().Name,
                    version.Major + "." + version.Minor + "." + version.Revision);

                // Initialize
                WinSparkle.Initialize();

                // Force a check
                WinSparkle.CheckUpdateWithUi();

                // Set the Can Shutdown callback
                WinSparkle.SetCanShutdownCallback(SetCanShutdownCallback_Callback);

                // Set the Shutdown callback
                WinSparkle.SetShutdownRequestCallback(SetShutdownRequestCallback_Callback);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Winsparkle Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private int SetCanShutdownCallback_Callback()
        {
            try
            {
                var result = MessageBox.Show("Do you wish to close the application and apply the update?", "Update now?",
                    MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes);
                return result == MessageBoxResult.Yes ? 1 : 0;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return 0;
            }
        }

        private void SetShutdownRequestCallback_Callback()
        {
            try
            {
                _context.Send(s => Application.Current.Shutdown(), null);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void OnClosed(object sender, EventArgs eventArgs)
        {
            try
            {
                // Clean up winsparkle
                WinSparkle.Cleanup();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}
