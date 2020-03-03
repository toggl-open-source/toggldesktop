using System.Windows.Controls;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class ProjectLabel : UserControl
    {
        public ProjectLabelViewModel ViewModel
        {
            get => (ProjectLabelViewModel)DataContext;
            set => DataContext = value;
        }

        public ProjectLabel()
        {
            InitializeComponent();
        }
    }
}