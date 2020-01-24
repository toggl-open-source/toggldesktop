using System.Windows.Media;
using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class ProjectViewModel : ReactiveObject
    {
        private string _projectName;
        public string ProjectName
        {
            get => _projectName;
            set => this.RaiseAndSetIfChanged(ref _projectName, value);
        }

        private string _taskName;
        public string TaskName
        {
            get => _taskName;
            set => this.RaiseAndSetIfChanged(ref _taskName, value);
        }

        private string _clientName;
        public string ClientName
        {
            get => _clientName;
            set => this.RaiseAndSetIfChanged(ref _clientName, value);
        }

        private Brush _color;
        public Brush Color
        {
            get => _color;
            set => this.RaiseAndSetIfChanged(ref _color, value);
        }

        public void Clear()
        {
            ProjectName = string.Empty;
            TaskName = string.Empty;
            ClientName = string.Empty;
            Color = default;
        }

        public void SetProject(Toggl.TogglTimeEntryView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            Color = Utils.ProjectColorBrushFromString(item.Color);
        }

        public void SetProject(Toggl.TogglAutocompleteView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            Color = Utils.ProjectColorBrushFromString(item.ProjectColor);
        }
    }
}