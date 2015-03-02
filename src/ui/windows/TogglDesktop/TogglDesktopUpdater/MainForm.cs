using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

        private void MainForm_Load(object sender, EventArgs e)
        {
            string[] args = Environment.GetCommandLineArgs();
            if (args.Length != 4) {
                MessageBox.Show(string.Format("Expected 4 arguments, got {0}", args.Length));
                return;
            }
            string pid = args[1];
            string installer = args[2];
            string executable = args[3];

            // FIXME: wait for pid to stop, then start installer

            // Run installer
            if (-1 == installer.IndexOf("TogglDesktopInstaller") ) {
                MessageBox.Show(string.Format("Unexpected installer name {0}", installer));
                return;
            }
            if (!installer.EndsWith(".exe")) {
                MessageBox.Show(string.Format("Unexpected installer extension {0}", installer));
                return;
            }

            Process process = Process.Start(installer, "/S");
            if (!process.WaitForExit(15 * 1000))
            {
                MessageBox.Show("The Toggl Desktop installer timed out");
                return;
            }

            System.IO.File.Delete(installer);

            Process.Start(executable);

            Environment.Exit(0);
        }
    }
}
