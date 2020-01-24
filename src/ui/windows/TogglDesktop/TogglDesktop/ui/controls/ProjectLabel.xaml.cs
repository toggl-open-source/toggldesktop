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

        public void Clear()
        {
            ViewModel.ProjectName = string.Empty;
            ViewModel.TaskName = string.Empty;
            ViewModel.ClientName = string.Empty;
            ViewModel.Color = default;
        }

        public void SetProject(Toggl.TogglTimeEntryView item)
        {
            ViewModel.ProjectName = item.ProjectLabel;
            ViewModel.TaskName = item.TaskLabel;
            ViewModel.ClientName = item.ClientLabel;
            ViewModel.Color = Utils.ProjectColorBrushFromString(item.Color);
        }

        public void SetProject(Toggl.TogglAutocompleteView item)
        {
            ViewModel.ProjectName = item.ProjectLabel;
            ViewModel.TaskName = item.TaskLabel;
            ViewModel.ClientName = item.ClientLabel;
            ViewModel.Color = Utils.ProjectColorBrushFromString(item.ProjectColor);
        }
    }
}