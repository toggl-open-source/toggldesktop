using System.Windows.Media;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class ProjectLabelViewModel : ReactiveObject
    {
        [Reactive]
        public string ProjectName { get; private set; }

        [Reactive]
        public string TaskName { get; private set; }

        [Reactive]
        public string ClientName { get; private set; }

        [Reactive]
        public Brush Color { get; private set; }

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