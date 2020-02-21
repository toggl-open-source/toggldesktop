using System.Reactive.Linq;
using System.Windows.Media;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class ProjectLabelViewModel : ReactiveObject
    {
        public ProjectLabelViewModel()
        {
            this.WhenAnyValue(x => x.ColorString)
                .Select(Utils.ProjectColorBrushFromString)
                .ToPropertyEx(this, x => x.Color);
        }

        [Reactive]
        public string ProjectName { get; private set; }

        [Reactive]
        public string TaskName { get; private set; }

        [Reactive]
        public string ClientName { get; private set; }

        [Reactive]
        public string ColorString { get; private set; }

        [Reactive]
        public string WorkspaceName { get; private set; }

        public Brush Color { [ObservableAsProperty] get; }

        public ProjectInfo ProjectInfo { get; private set; }

        public bool HasProject => ProjectInfo.ProjectId != default;

        public void Clear()
        {
            ProjectName = string.Empty;
            TaskName = string.Empty;
            ClientName = string.Empty;
            ColorString = string.Empty;
            WorkspaceName = string.Empty;
            ProjectInfo = default;
        }

        public void SetProject(Toggl.TogglTimeEntryView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            ColorString = item.Color;
            WorkspaceName = item.WorkspaceName;
            ProjectInfo = new ProjectInfo(item.PID, item.TID);
        }

        public void SetProject(Toggl.TogglAutocompleteView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            ColorString = item.ProjectColor;
            WorkspaceName = item.WorkspaceName;
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