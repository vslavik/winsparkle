using System;
using System.Windows.Forms;
using WinSparkleDotNet;

namespace example_dotnet
{
    public partial class WinSparkleDemoForm : Form
    {
        private readonly WinSparkleNet _sparkleNet;

        public WinSparkleDemoForm()
        {
            InitializeComponent();
            try
            {
                _sparkleNet = new WinSparkleNet();
            }
            catch (Exception ex)
            {
                MessageBox.Show(@"Error was : "+ex, @"Error during SparkleNet instantiation",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            //Callback
            _sparkleNet.SetCanShutdownCallback(this.CanShutDownCallback);
            _sparkleNet.SetShutdownRequestCallback(this.ShutDownRequestCallback);
            _sparkleNet.SetDidFindUpdateCallback(this.DidFindAnUpdateCallback);
            _sparkleNet.SetDidNotFindUpdateCallback(this.DidNotFindAnUpdateCallback);
            _sparkleNet.SetUpdateCancelledCallback(this.UpdateCancelledCallback);
            _sparkleNet.SetErrorCallback(this.ErrorCallback);
            //Init
            _sparkleNet.Initialize();
            _sparkleNet.CheckForUpdate();
        }

        private void Form_FormClosed(object sender, FormClosedEventArgs e)
        {
            _sparkleNet.Cleanup();
        }

        private void Form_Load(object sender, System.EventArgs e)
        {
            txtb_lastChecked.Text = _sparkleNet.LastCheckTime.ToString("dd'/'MM'/'yyyy HH:mm:ss");
            txtb_updateInterval.Text = _sparkleNet.UpdateInterval.Seconds.ToString();
            ckb_autoCheckUpdate.Checked = _sparkleNet.AutomaticCheckForUpdates;
            llbl_appCastUrl.Text = _sparkleNet.AppCastUrl;
        }

        private void btn_checkUpdate_Click(object sender, System.EventArgs e)
        {
            _sparkleNet.CheckForUpdate();
        }

        private void btn_checkUpdateNoUI_Click(object sender, EventArgs e)
        {
            _sparkleNet.CheckForUpdate(false);
        }

        private void btn_checkUpdateAndInstall_Click(object sender, EventArgs e)
        {
            _sparkleNet.CheckForUpdate(true, true);
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

        private void DidFindAnUpdateCallback()
        {
            try
            {
                MessageBox.Show(@"Callback called if an update has been found.", @"DidFindUpdateCallback",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void DidNotFindAnUpdateCallback()
        {
            try
            {
                MessageBox.Show(@"Callback called if an update hasn't been found.", @"DidNotFindUpdateCallback",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void UpdateCancelledCallback()
        {
            try
            {
                MessageBox.Show(@"Callback called if an update has been cancelled/ignored by user.", @"UpdateCancelledCallback",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void ErrorCallback()
        {
            try
            {
                MessageBox.Show(@"Callback called if WinSparkle get an error.", @"ErrorCallback",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void ShutDownRequestCallback()
        {
            try
            {
                if (System.Windows.Forms.Application.MessageLoop)
                {
                    System.Windows.Forms.Application.Exit();
                }
                else
                {
                    System.Environment.Exit(1);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, @"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
