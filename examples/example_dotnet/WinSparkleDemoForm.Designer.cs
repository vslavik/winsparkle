namespace example_dotnet
{
    partial class WinSparkleDemoForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btn_checkUpdate = new System.Windows.Forms.Button();
            this.gp_infos = new System.Windows.Forms.GroupBox();
            this.lbl_appCastUrl = new System.Windows.Forms.Label();
            this.llbl_appCastUrl = new System.Windows.Forms.LinkLabel();
            this.ckb_autoCheckUpdate = new System.Windows.Forms.CheckBox();
            this.lbl_autoCheckUpdate = new System.Windows.Forms.Label();
            this.txtb_updateInterval = new System.Windows.Forms.TextBox();
            this.lbl_updateInterval = new System.Windows.Forms.Label();
            this.txtb_lastChecked = new System.Windows.Forms.TextBox();
            this.lbl_lastChecked = new System.Windows.Forms.Label();
            this.btn_close = new System.Windows.Forms.Button();
            this.btn_checkUpdateAndInstall = new System.Windows.Forms.Button();
            this.btn_checkUpdateNoUI = new System.Windows.Forms.Button();
            this.gp_infos.SuspendLayout();
            this.SuspendLayout();
            // 
            // btn_checkUpdate
            // 
            this.btn_checkUpdate.Location = new System.Drawing.Point(285, 132);
            this.btn_checkUpdate.Name = "btn_checkUpdate";
            this.btn_checkUpdate.Size = new System.Drawing.Size(128, 23);
            this.btn_checkUpdate.TabIndex = 0;
            this.btn_checkUpdate.Text = "Check Update";
            this.btn_checkUpdate.UseVisualStyleBackColor = true;
            this.btn_checkUpdate.Click += new System.EventHandler(this.btn_checkUpdate_Click);
            // 
            // gp_infos
            // 
            this.gp_infos.Controls.Add(this.lbl_appCastUrl);
            this.gp_infos.Controls.Add(this.llbl_appCastUrl);
            this.gp_infos.Controls.Add(this.ckb_autoCheckUpdate);
            this.gp_infos.Controls.Add(this.lbl_autoCheckUpdate);
            this.gp_infos.Controls.Add(this.txtb_updateInterval);
            this.gp_infos.Controls.Add(this.lbl_updateInterval);
            this.gp_infos.Controls.Add(this.txtb_lastChecked);
            this.gp_infos.Controls.Add(this.lbl_lastChecked);
            this.gp_infos.Location = new System.Drawing.Point(12, 12);
            this.gp_infos.Name = "gp_infos";
            this.gp_infos.Size = new System.Drawing.Size(267, 201);
            this.gp_infos.TabIndex = 1;
            this.gp_infos.TabStop = false;
            this.gp_infos.Text = "Infos";
            // 
            // lbl_appCastUrl
            // 
            this.lbl_appCastUrl.AutoSize = true;
            this.lbl_appCastUrl.Location = new System.Drawing.Point(6, 108);
            this.lbl_appCastUrl.Name = "lbl_appCastUrl";
            this.lbl_appCastUrl.Size = new System.Drawing.Size(67, 13);
            this.lbl_appCastUrl.TabIndex = 8;
            this.lbl_appCastUrl.Text = "appcast Url :";
            // 
            // llbl_appCastUrl
            // 
            this.llbl_appCastUrl.AutoSize = true;
            this.llbl_appCastUrl.Location = new System.Drawing.Point(6, 130);
            this.llbl_appCastUrl.Name = "llbl_appCastUrl";
            this.llbl_appCastUrl.Size = new System.Drawing.Size(78, 13);
            this.llbl_appCastUrl.TabIndex = 7;
            this.llbl_appCastUrl.TabStop = true;
            this.llbl_appCastUrl.Text = "APPCASTURL";
            // 
            // ckb_autoCheckUpdate
            // 
            this.ckb_autoCheckUpdate.AutoSize = true;
            this.ckb_autoCheckUpdate.Enabled = false;
            this.ckb_autoCheckUpdate.Location = new System.Drawing.Point(106, 84);
            this.ckb_autoCheckUpdate.Name = "ckb_autoCheckUpdate";
            this.ckb_autoCheckUpdate.Size = new System.Drawing.Size(15, 14);
            this.ckb_autoCheckUpdate.TabIndex = 5;
            this.ckb_autoCheckUpdate.UseVisualStyleBackColor = true;
            // 
            // lbl_autoCheckUpdate
            // 
            this.lbl_autoCheckUpdate.AutoSize = true;
            this.lbl_autoCheckUpdate.Location = new System.Drawing.Point(6, 85);
            this.lbl_autoCheckUpdate.Name = "lbl_autoCheckUpdate";
            this.lbl_autoCheckUpdate.Size = new System.Drawing.Size(94, 13);
            this.lbl_autoCheckUpdate.TabIndex = 4;
            this.lbl_autoCheckUpdate.Text = "Automatic Check :";
            // 
            // txtb_updateInterval
            // 
            this.txtb_updateInterval.Location = new System.Drawing.Point(90, 58);
            this.txtb_updateInterval.Name = "txtb_updateInterval";
            this.txtb_updateInterval.ReadOnly = true;
            this.txtb_updateInterval.Size = new System.Drawing.Size(171, 20);
            this.txtb_updateInterval.TabIndex = 3;
            // 
            // lbl_updateInterval
            // 
            this.lbl_updateInterval.AutoSize = true;
            this.lbl_updateInterval.Location = new System.Drawing.Point(6, 61);
            this.lbl_updateInterval.Name = "lbl_updateInterval";
            this.lbl_updateInterval.Size = new System.Drawing.Size(86, 13);
            this.lbl_updateInterval.TabIndex = 2;
            this.lbl_updateInterval.Text = "Update Interval :";
            // 
            // txtb_lastChecked
            // 
            this.txtb_lastChecked.Location = new System.Drawing.Point(90, 27);
            this.txtb_lastChecked.Name = "txtb_lastChecked";
            this.txtb_lastChecked.ReadOnly = true;
            this.txtb_lastChecked.Size = new System.Drawing.Size(171, 20);
            this.txtb_lastChecked.TabIndex = 1;
            // 
            // lbl_lastChecked
            // 
            this.lbl_lastChecked.AutoSize = true;
            this.lbl_lastChecked.Location = new System.Drawing.Point(6, 30);
            this.lbl_lastChecked.Name = "lbl_lastChecked";
            this.lbl_lastChecked.Size = new System.Drawing.Size(78, 13);
            this.lbl_lastChecked.TabIndex = 0;
            this.lbl_lastChecked.Text = "Last checked :";
            // 
            // btn_close
            // 
            this.btn_close.Location = new System.Drawing.Point(419, 190);
            this.btn_close.Name = "btn_close";
            this.btn_close.Size = new System.Drawing.Size(132, 23);
            this.btn_close.TabIndex = 2;
            this.btn_close.Text = "Close";
            this.btn_close.UseVisualStyleBackColor = true;
            this.btn_close.Click += new System.EventHandler(this.btn_close_Click);
            // 
            // btn_checkUpdateAndInstall
            // 
            this.btn_checkUpdateAndInstall.Location = new System.Drawing.Point(285, 161);
            this.btn_checkUpdateAndInstall.Name = "btn_checkUpdateAndInstall";
            this.btn_checkUpdateAndInstall.Size = new System.Drawing.Size(266, 23);
            this.btn_checkUpdateAndInstall.TabIndex = 3;
            this.btn_checkUpdateAndInstall.Text = "Check Update and Install";
            this.btn_checkUpdateAndInstall.UseVisualStyleBackColor = true;
            this.btn_checkUpdateAndInstall.Click += new System.EventHandler(this.btn_checkUpdateAndInstall_Click);
            // 
            // btn_checkUpdateNoUI
            // 
            this.btn_checkUpdateNoUI.Location = new System.Drawing.Point(419, 132);
            this.btn_checkUpdateNoUI.Name = "btn_checkUpdateNoUI";
            this.btn_checkUpdateNoUI.Size = new System.Drawing.Size(132, 23);
            this.btn_checkUpdateNoUI.TabIndex = 4;
            this.btn_checkUpdateNoUI.Text = "Check Update no UI";
            this.btn_checkUpdateNoUI.UseVisualStyleBackColor = true;
            this.btn_checkUpdateNoUI.Click += new System.EventHandler(this.btn_checkUpdateNoUI_Click);
            // 
            // WinSparkleDemoForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(563, 225);
            this.Controls.Add(this.btn_checkUpdateNoUI);
            this.Controls.Add(this.btn_checkUpdateAndInstall);
            this.Controls.Add(this.btn_close);
            this.Controls.Add(this.gp_infos);
            this.Controls.Add(this.btn_checkUpdate);
            this.Name = "WinSparkleDemoForm";
            this.Text = "WinSparkle - DotNet - Example";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form_FormClosed);
            this.Load += new System.EventHandler(this.Form_Load);
            this.gp_infos.ResumeLayout(false);
            this.gp_infos.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btn_checkUpdate;
        private System.Windows.Forms.GroupBox gp_infos;
        private System.Windows.Forms.Label lbl_lastChecked;
        private System.Windows.Forms.TextBox txtb_lastChecked;
        private System.Windows.Forms.TextBox txtb_updateInterval;
        private System.Windows.Forms.Label lbl_updateInterval;
        private System.Windows.Forms.Label lbl_autoCheckUpdate;
        private System.Windows.Forms.CheckBox ckb_autoCheckUpdate;
        private System.Windows.Forms.Label lbl_appCastUrl;
        private System.Windows.Forms.LinkLabel llbl_appCastUrl;
        private System.Windows.Forms.Button btn_close;
        private System.Windows.Forms.Button btn_checkUpdateAndInstall;
        private System.Windows.Forms.Button btn_checkUpdateNoUI;
    }
}

