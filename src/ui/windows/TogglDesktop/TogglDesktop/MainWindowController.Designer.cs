namespace TogglDesktop
{
    partial class MainWindowController
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindowController));
            this.troubleBox = new System.Windows.Forms.Panel();
            this.buttonDismissError = new System.Windows.Forms.Button();
            this.errorLabel = new System.Windows.Forms.Label();
            this.trayIcon = new System.Windows.Forms.NotifyIcon(this.components);
            this.trayIconMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.runningToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.continueToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.stopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.showToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.syncToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openInBrowserToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.preferencesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
            this.clearCacheToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sendFeedbackToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.logoutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.timerIdleDetection = new System.Windows.Forms.Timer(this.components);
            this.mainMenuStrip = new System.Windows.Forms.MenuStrip();
            this.togglDesktopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuNew = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuContinue = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuStop = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
            this.mainMenuSync = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuReports = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuPreferences = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripSeparator();
            this.mainMenuClearCache = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuSendFeedback = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuAbout = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuLogout = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuQuit = new System.Windows.Forms.ToolStripMenuItem();
            this.contentPanel = new System.Windows.Forms.Panel();
            this.troubleBox.SuspendLayout();
            this.trayIconMenu.SuspendLayout();
            this.mainMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // troubleBox
            // 
            this.troubleBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.troubleBox.Controls.Add(this.buttonDismissError);
            this.troubleBox.Controls.Add(this.errorLabel);
            this.troubleBox.Location = new System.Drawing.Point(0, 0);
            this.troubleBox.Name = "troubleBox";
            this.troubleBox.Size = new System.Drawing.Size(299, 30);
            this.troubleBox.TabIndex = 5;
            this.troubleBox.Visible = false;
            // 
            // buttonDismissError
            // 
            this.buttonDismissError.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonDismissError.Location = new System.Drawing.Point(280, 4);
            this.buttonDismissError.Name = "buttonDismissError";
            this.buttonDismissError.Size = new System.Drawing.Size(16, 23);
            this.buttonDismissError.TabIndex = 6;
            this.buttonDismissError.Text = "X";
            this.buttonDismissError.UseVisualStyleBackColor = true;
            this.buttonDismissError.Click += new System.EventHandler(this.buttonDismissError_Click);
            // 
            // errorLabel
            // 
            this.errorLabel.AutoSize = true;
            this.errorLabel.Font = new System.Drawing.Font("Lucida Sans Unicode", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.errorLabel.Location = new System.Drawing.Point(3, 9);
            this.errorLabel.Name = "errorLabel";
            this.errorLabel.Size = new System.Drawing.Size(78, 16);
            this.errorLabel.TabIndex = 5;
            this.errorLabel.Text = "Login failed!";
            // 
            // trayIcon
            // 
            this.trayIcon.ContextMenuStrip = this.trayIconMenu;
            this.trayIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("trayIcon.Icon")));
            this.trayIcon.Visible = true;
            this.trayIcon.BalloonTipClicked += new System.EventHandler(this.trayIcon_BalloonTipClicked);
            this.trayIcon.MouseClick += new System.Windows.Forms.MouseEventHandler(this.trayIcon_MouseClick);
            // 
            // trayIconMenu
            // 
            this.trayIconMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.runningToolStripMenuItem,
            this.toolStripMenuItem1,
            this.newToolStripMenuItem,
            this.continueToolStripMenuItem,
            this.stopToolStripMenuItem,
            this.toolStripMenuItem2,
            this.showToolStripMenuItem,
            this.toolStripMenuItem3,
            this.syncToolStripMenuItem,
            this.openInBrowserToolStripMenuItem,
            this.preferencesToolStripMenuItem,
            this.toolStripMenuItem4,
            this.clearCacheToolStripMenuItem,
            this.sendFeedbackToolStripMenuItem,
            this.aboutToolStripMenuItem,
            this.logoutToolStripMenuItem,
            this.quitToolStripMenuItem});
            this.trayIconMenu.Name = "trayIconMenu";
            this.trayIconMenu.Size = new System.Drawing.Size(154, 314);
            // 
            // runningToolStripMenuItem
            // 
            this.runningToolStripMenuItem.Name = "runningToolStripMenuItem";
            this.runningToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.runningToolStripMenuItem.Text = "Running:";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(150, 6);
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.newToolStripMenuItem.Text = "New";
            this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // continueToolStripMenuItem
            // 
            this.continueToolStripMenuItem.Name = "continueToolStripMenuItem";
            this.continueToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.continueToolStripMenuItem.Text = "Continue";
            this.continueToolStripMenuItem.Click += new System.EventHandler(this.continueToolStripMenuItem_Click);
            // 
            // stopToolStripMenuItem
            // 
            this.stopToolStripMenuItem.Name = "stopToolStripMenuItem";
            this.stopToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.stopToolStripMenuItem.Text = "Stop";
            this.stopToolStripMenuItem.Click += new System.EventHandler(this.stopToolStripMenuItem_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(150, 6);
            // 
            // showToolStripMenuItem
            // 
            this.showToolStripMenuItem.Name = "showToolStripMenuItem";
            this.showToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.showToolStripMenuItem.Text = "Show";
            this.showToolStripMenuItem.Click += new System.EventHandler(this.showToolStripMenuItem_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(150, 6);
            // 
            // syncToolStripMenuItem
            // 
            this.syncToolStripMenuItem.Name = "syncToolStripMenuItem";
            this.syncToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.syncToolStripMenuItem.Text = "Sync";
            this.syncToolStripMenuItem.Click += new System.EventHandler(this.syncToolStripMenuItem_Click);
            // 
            // openInBrowserToolStripMenuItem
            // 
            this.openInBrowserToolStripMenuItem.Name = "openInBrowserToolStripMenuItem";
            this.openInBrowserToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.openInBrowserToolStripMenuItem.Text = "Reports";
            this.openInBrowserToolStripMenuItem.Click += new System.EventHandler(this.openInBrowserToolStripMenuItem_Click);
            // 
            // preferencesToolStripMenuItem
            // 
            this.preferencesToolStripMenuItem.Name = "preferencesToolStripMenuItem";
            this.preferencesToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.preferencesToolStripMenuItem.Text = "Preferences";
            this.preferencesToolStripMenuItem.Click += new System.EventHandler(this.preferencesToolStripMenuItem_Click);
            // 
            // toolStripMenuItem4
            // 
            this.toolStripMenuItem4.Name = "toolStripMenuItem4";
            this.toolStripMenuItem4.Size = new System.Drawing.Size(150, 6);
            // 
            // clearCacheToolStripMenuItem
            // 
            this.clearCacheToolStripMenuItem.Name = "clearCacheToolStripMenuItem";
            this.clearCacheToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.clearCacheToolStripMenuItem.Text = "Clear Cache";
            this.clearCacheToolStripMenuItem.Click += new System.EventHandler(this.clearCacheToolStripMenuItem_Click);
            // 
            // sendFeedbackToolStripMenuItem
            // 
            this.sendFeedbackToolStripMenuItem.Name = "sendFeedbackToolStripMenuItem";
            this.sendFeedbackToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.sendFeedbackToolStripMenuItem.Text = "Send Feedback";
            this.sendFeedbackToolStripMenuItem.Click += new System.EventHandler(this.sendFeedbackToolStripMenuItem_Click);
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // logoutToolStripMenuItem
            // 
            this.logoutToolStripMenuItem.Name = "logoutToolStripMenuItem";
            this.logoutToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.logoutToolStripMenuItem.Text = "Logout";
            this.logoutToolStripMenuItem.Click += new System.EventHandler(this.logoutToolStripMenuItem_Click);
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(153, 22);
            this.quitToolStripMenuItem.Text = "Quit";
            this.quitToolStripMenuItem.Click += new System.EventHandler(this.quitToolStripMenuItem_Click);
            // 
            // timerIdleDetection
            // 
            this.timerIdleDetection.Interval = 1000;
            this.timerIdleDetection.Tick += new System.EventHandler(this.timerIdleDetection_Tick);
            // 
            // mainMenuStrip
            // 
            this.mainMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.togglDesktopToolStripMenuItem});
            this.mainMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.mainMenuStrip.Name = "mainMenuStrip";
            this.mainMenuStrip.Size = new System.Drawing.Size(299, 24);
            this.mainMenuStrip.TabIndex = 6;
            this.mainMenuStrip.Text = "menuStrip1";
            // 
            // togglDesktopToolStripMenuItem
            // 
            this.togglDesktopToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mainMenuNew,
            this.mainMenuContinue,
            this.mainMenuStop,
            this.toolStripMenuItem5,
            this.mainMenuSync,
            this.mainMenuReports,
            this.mainMenuPreferences,
            this.toolStripMenuItem6,
            this.mainMenuClearCache,
            this.mainMenuSendFeedback,
            this.mainMenuAbout,
            this.mainMenuLogout,
            this.mainMenuQuit});
            this.togglDesktopToolStripMenuItem.Name = "togglDesktopToolStripMenuItem";
            this.togglDesktopToolStripMenuItem.Size = new System.Drawing.Size(96, 20);
            this.togglDesktopToolStripMenuItem.Text = "Toggl Desktop";
            // 
            // mainMenuNew
            // 
            this.mainMenuNew.Name = "mainMenuNew";
            this.mainMenuNew.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.N)));
            this.mainMenuNew.Size = new System.Drawing.Size(161, 22);
            this.mainMenuNew.Text = "New";
            this.mainMenuNew.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // mainMenuContinue
            // 
            this.mainMenuContinue.Name = "mainMenuContinue";
            this.mainMenuContinue.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.C)));
            this.mainMenuContinue.Size = new System.Drawing.Size(161, 22);
            this.mainMenuContinue.Text = "Continue";
            this.mainMenuContinue.Click += new System.EventHandler(this.continueToolStripMenuItem_Click);
            // 
            // mainMenuStop
            // 
            this.mainMenuStop.Name = "mainMenuStop";
            this.mainMenuStop.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.S)));
            this.mainMenuStop.Size = new System.Drawing.Size(161, 22);
            this.mainMenuStop.Text = "Stop";
            this.mainMenuStop.Click += new System.EventHandler(this.stopToolStripMenuItem_Click);
            // 
            // toolStripMenuItem5
            // 
            this.toolStripMenuItem5.Name = "toolStripMenuItem5";
            this.toolStripMenuItem5.Size = new System.Drawing.Size(158, 6);
            // 
            // mainMenuSync
            // 
            this.mainMenuSync.Name = "mainMenuSync";
            this.mainMenuSync.Size = new System.Drawing.Size(161, 22);
            this.mainMenuSync.Text = "Sync";
            this.mainMenuSync.Click += new System.EventHandler(this.syncToolStripMenuItem_Click);
            // 
            // mainMenuReports
            // 
            this.mainMenuReports.Name = "mainMenuReports";
            this.mainMenuReports.Size = new System.Drawing.Size(161, 22);
            this.mainMenuReports.Text = "Reports";
            this.mainMenuReports.Click += new System.EventHandler(this.openInBrowserToolStripMenuItem_Click);
            // 
            // mainMenuPreferences
            // 
            this.mainMenuPreferences.Name = "mainMenuPreferences";
            this.mainMenuPreferences.Size = new System.Drawing.Size(161, 22);
            this.mainMenuPreferences.Text = "Preferences";
            this.mainMenuPreferences.Click += new System.EventHandler(this.preferencesToolStripMenuItem_Click);
            // 
            // toolStripMenuItem6
            // 
            this.toolStripMenuItem6.Name = "toolStripMenuItem6";
            this.toolStripMenuItem6.Size = new System.Drawing.Size(158, 6);
            // 
            // mainMenuClearCache
            // 
            this.mainMenuClearCache.Name = "mainMenuClearCache";
            this.mainMenuClearCache.Size = new System.Drawing.Size(161, 22);
            this.mainMenuClearCache.Text = "Clear Cache";
            this.mainMenuClearCache.Click += new System.EventHandler(this.clearCacheToolStripMenuItem_Click);
            // 
            // mainMenuSendFeedback
            // 
            this.mainMenuSendFeedback.Name = "mainMenuSendFeedback";
            this.mainMenuSendFeedback.Size = new System.Drawing.Size(161, 22);
            this.mainMenuSendFeedback.Text = "Send Feedback";
            this.mainMenuSendFeedback.Click += new System.EventHandler(this.sendFeedbackToolStripMenuItem_Click);
            // 
            // mainMenuAbout
            // 
            this.mainMenuAbout.Name = "mainMenuAbout";
            this.mainMenuAbout.Size = new System.Drawing.Size(161, 22);
            this.mainMenuAbout.Text = "About";
            this.mainMenuAbout.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // mainMenuLogout
            // 
            this.mainMenuLogout.Name = "mainMenuLogout";
            this.mainMenuLogout.Size = new System.Drawing.Size(161, 22);
            this.mainMenuLogout.Text = "Logout";
            this.mainMenuLogout.Click += new System.EventHandler(this.logoutToolStripMenuItem_Click);
            // 
            // mainMenuQuit
            // 
            this.mainMenuQuit.Name = "mainMenuQuit";
            this.mainMenuQuit.Size = new System.Drawing.Size(161, 22);
            this.mainMenuQuit.Text = "Quit";
            this.mainMenuQuit.Click += new System.EventHandler(this.quitToolStripMenuItem_Click);
            // 
            // contentPanel
            // 
            this.contentPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.contentPanel.Location = new System.Drawing.Point(0, 24);
            this.contentPanel.Name = "contentPanel";
            this.contentPanel.Size = new System.Drawing.Size(299, 298);
            this.contentPanel.TabIndex = 7;
            // 
            // MainWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(299, 322);
            this.Controls.Add(this.contentPanel);
            this.Controls.Add(this.mainMenuStrip);
            this.Controls.Add(this.troubleBox);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.mainMenuStrip;
            this.MinimumSize = new System.Drawing.Size(315, 360);
            this.Name = "MainWindowController";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Toggl Desktop";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainWindowController_FormClosing);
            this.Load += new System.EventHandler(this.MainWindowController_Load);
            this.troubleBox.ResumeLayout(false);
            this.troubleBox.PerformLayout();
            this.trayIconMenu.ResumeLayout(false);
            this.mainMenuStrip.ResumeLayout(false);
            this.mainMenuStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel troubleBox;
        private System.Windows.Forms.Label errorLabel;
        private System.Windows.Forms.Button buttonDismissError;
        private System.Windows.Forms.NotifyIcon trayIcon;
        private System.Windows.Forms.ContextMenuStrip trayIconMenu;
        private System.Windows.Forms.ToolStripMenuItem runningToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem continueToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem stopToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem showToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem syncToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openInBrowserToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem preferencesToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem sendFeedbackToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem logoutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem quitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearCacheToolStripMenuItem;
        private System.Windows.Forms.Timer timerIdleDetection;
        private System.Windows.Forms.MenuStrip mainMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem togglDesktopToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mainMenuNew;
        private System.Windows.Forms.ToolStripMenuItem mainMenuContinue;
        private System.Windows.Forms.ToolStripMenuItem mainMenuStop;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
        private System.Windows.Forms.ToolStripMenuItem mainMenuSync;
        private System.Windows.Forms.ToolStripMenuItem mainMenuReports;
        private System.Windows.Forms.ToolStripMenuItem mainMenuPreferences;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem6;
        private System.Windows.Forms.ToolStripMenuItem mainMenuClearCache;
        private System.Windows.Forms.ToolStripMenuItem mainMenuSendFeedback;
        private System.Windows.Forms.ToolStripMenuItem mainMenuAbout;
        private System.Windows.Forms.ToolStripMenuItem mainMenuLogout;
        private System.Windows.Forms.ToolStripMenuItem mainMenuQuit;
        private System.Windows.Forms.Panel contentPanel;
    }
}

