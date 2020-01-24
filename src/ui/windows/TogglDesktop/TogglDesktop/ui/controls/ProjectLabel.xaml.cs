using System.Windows.Controls;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class ProjectLabel : UserControl
    {
        public ProjectViewModel ViewModel
        {
            get => (ProjectViewModel)DataContext;
            set => DataContext = value;
        }

        public ProjectLabel()
        {
            InitializeComponent();
            ViewModel = new ProjectViewModel();
        }
    }
}