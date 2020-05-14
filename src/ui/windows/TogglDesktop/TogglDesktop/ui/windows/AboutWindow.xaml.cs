using System.Windows;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class AboutWindow
    {
        public AboutWindowViewModel ViewModel
        {
            get => (AboutWindowViewModel)DataContext;
            set => DataContext = value;
        }

        public AboutWindow(AboutWindowViewModel viewModel)
        {
            ViewModel = viewModel;
            this.InitializeComponent();
        }

        private void onGithubLinkClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser("https://github.com/toggl-open-source/toggldesktop");
        }
    }
}
