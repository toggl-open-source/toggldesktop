using System.Windows.Controls;
using TogglTrack.ViewModels;

namespace TogglTrack
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