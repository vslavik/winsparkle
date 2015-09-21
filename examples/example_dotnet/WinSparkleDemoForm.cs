using System;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;
using WinSparkleDotNet;

namespace example_dotnet
{
    public partial class WinSparkleDemoForm : Form
    {
        private SynchronizationContext _context;
        private static string _appCastURL = @"http://winsparkle.org/example/appcast.xml";

        public WinSparkleDemoForm()
        {
            _context = SynchronizationContext.Current;
            InitializeComponent();
            
            var assembly = Assembly.GetExecutingAssembly();
            var company = (AssemblyCompanyAttribute)assembly.GetCustomAttributes(typeof(AssemblyCompanyAttribute), false)[0];
            var name = assembly.GetName().Name;
            var version = assembly.GetName().Version; //Sparkle can parse basic version scheme like X.X.X.X

            WinSparkle.SetAppcastUrl(_appCastURL);
            WinSparkle.SetAppDetails(company.Company, name, version.ToString());
            WinSparkle.SetCanShutdownCallback(CanShutDownCallback);
            WinSparkle.SetShutdownRequestCallback(ShutDownRequestCallback);
            WinSparkle.Initialize();
            WinSparkle.CheckUpdateWithUI();
        }

        private void Form_FormClosed(object sender, FormClosedEventArgs e)
        {
            //WinSparkle.Cleanup();
        }

        private void Form_Load(object sender, System.EventArgs e)
        {
            txtb_lastChecked.Text = WinSparkle.LastCheckTime.ToString("dd'/'MM'/'yyyy HH:mm:ss");
            txtb_updateInterval.Text = WinSparkle.UpdateInterval.Seconds.ToString();
            ckb_autoCheckUpdate.Checked = WinSparkle.AutomaticCheckForUpdates;
            llbl_appCastUrl.Text = _appCastURL;
        }

        private void btn_checkUpdate_Click(object sender, System.EventArgs e)
        {
            WinSparkle.CheckUpdateWithUI();
        }

        private void btn_close_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private bool CanShutDownCallback()
        {
            try
            {
                var result = MessageBox.Show(@"Here you can simulate if the application can be closed or not to proceed the update", @"CanShutDownCallback",
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                return result == DialogResult.Yes;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
        }

        private void ShutDownRequestCallback()
        {
            try
            {
                _context.Send(s => Application.Exit(), null);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
