using System;
using System.Linq;
using System.Windows;

namespace TogglDesktop
{
    partial class App
    {
        public App()
        {
            InitializeComponent();
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            var mainWindow = new MainWindow();
            var startMinimized = Environment.GetCommandLineArgs().Contains("--minimize");
            if (!startMinimized)
            {
                mainWindow.Show();
            }
        }
    }
}