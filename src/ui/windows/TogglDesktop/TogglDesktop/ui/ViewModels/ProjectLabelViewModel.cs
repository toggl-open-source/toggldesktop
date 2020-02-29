using System.Windows.Media;

namespace TogglDesktop.ViewModels
{
    public class ProjectLabelViewModel
    {
        public ProjectLabelViewModel(
            string projectName,
            string taskName,
            string clientName,
            string colorString,
            string workspaceName,
            ulong projectId,
            ulong taskId)
        {
            ProjectName = projectName;
            TaskName = taskName;
            ClientName = clientName;
            ColorString = colorString;
            WorkspaceName = workspaceName;
            ProjectInfo = new ProjectInfo(projectId, taskId);
            Color = Utils.ProjectColorBrushFromString(ColorString);
        }

        public string ProjectName { get; }

        public string TaskName { get; }

        public string ClientName { get; }

        public string ColorString { get; }

        public string WorkspaceName { get; }

        public Brush Color { get; }

        public ProjectInfo ProjectInfo { get; }

        public bool HasProject => ProjectInfo.ProjectId != default;
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