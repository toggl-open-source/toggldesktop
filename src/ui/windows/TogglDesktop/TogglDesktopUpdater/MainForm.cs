﻿using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Diagnostics;

namespace TogglDesktopUpdater
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        private string installer;
        private string pid;
        private string executable;

        private void MainForm_Load(object sender, EventArgs e)
        {
            try
            {
                upgrade();
                cleanup();
                Environment.Exit(0);
            }
            catch (Exception ex)
            {
                String error = ex.Message;
                if (error == "Access is denied")
                {
                    error = "Unable to automatically start Toggl Desktop. Please start Toggl Desktop manually";
                }
                MessageBox.Show(error, "Toggl Desktop update failed");
                Program.NotifyBugsnag(ex);
                cleanup();
                Environment.Exit(1);
            }
        }

        private void updateBugsnagMetadata(string pid, string installer, string executable)
        {
            Program.UpdateBugsnagMetadata("Installer", new Dictionary<string, string>
            {
                { "installer", installer },
                { "pid", pid },
                {"executable", executable }
            });
        }

        private void upgrade() 
        {
            string[] args = Environment.GetCommandLineArgs();
            if (args.Length != 4)
            {
                throw new ArgumentException(string.Format("Expected 4 arguments, got {0}", args.Length));
            }
            
            pid = args[1];
            installer = args[2];
            executable = args[3];

            updateBugsnagMetadata(pid, installer, executable);

            // should we wait for pid to stop, then start installer?

            // Run installer
            if (-1 == installer.IndexOf("TogglDesktopInstaller"))
            {
                throw new Exception(string.Format("Unexpected installer name {0}", installer));
            }

            if (!installer.EndsWith(".exe"))
            {
                throw new Exception(string.Format("Unexpected installer extension {0}", installer));
            }

            Process process = Process.Start(installer, "/S");
            if (!process.WaitForExit(15 * 1000))
            {
                throw new Exception("The Toggl Desktop installer timed out");
            }

            Process.Start(executable);
        }

        private void cleanup()
        {
            try
            {
                if (null != installer && 0 != installer.Length)
                {
                    System.IO.File.Delete(installer);
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Error deleting installer: " + ex.Message);
                Program.NotifyBugsnag(ex);
            }
        }
    }
}
