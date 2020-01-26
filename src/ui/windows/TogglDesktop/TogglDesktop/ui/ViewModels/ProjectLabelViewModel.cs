using System.Windows.Media;
using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class ProjectLabelViewModel : ReactiveObject
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

        public ProjectInfo ProjectInfo { get; private set; }

        public bool HasProject => ProjectInfo.ProjectId != default;

        public void Clear()
        {
            ProjectName = string.Empty;
            TaskName = string.Empty;
            ClientName = string.Empty;
            Color = default;
            ProjectInfo = default;
        }

        public void SetProject(Toggl.TogglTimeEntryView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            Color = Utils.ProjectColorBrushFromString(item.Color);
            ProjectInfo = new ProjectInfo(item.PID, item.TID);
        }

        public void SetProject(Toggl.TogglAutocompleteView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            Color = Utils.ProjectColorBrushFromString(item.ProjectColor);
            ProjectInfo = new ProjectInfo(item.ProjectID, item.TaskID);
        }
    }

    public struct ProjectInfo
    {
        public ulong ProjectId { get; }
        public ulong TaskId { get; }

        public ProjectInfo(ulong projectId, ulong taskId)
        {
            this.ProjectId = projectId;
            this.TaskId = taskId;
        }
    }
}