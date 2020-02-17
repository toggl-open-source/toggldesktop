using System.Reactive.Linq;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimeEntryLabelViewModel : ReactiveObject
    {
        public TimeEntryLabelViewModel(ProjectLabelViewModel projectLabel)
        {
            ProjectLabel = projectLabel;
            this.WhenAnyValue(x => x.ProjectLabel.ProjectName)
                .Select(projectName => string.IsNullOrEmpty(projectName) ? "+ Add details" : "+ Add description")
                .ToPropertyEx(this, x => x.AddDescriptionLabelText);
            this.WhenAnyValue(x => x.Description, x => x.ProjectLabel.ProjectName)
                .Select(((string description, string projectName) tuple) => GetIsAddProjectLabelVisible(tuple.description, tuple.projectName))
                .ToPropertyEx(this, x => x.IsAddProjectLabelVisible);
        }

        public ProjectLabelViewModel ProjectLabel { get; }

        [Reactive]
        public string Description { get; private set; }

        public string AddDescriptionLabelText { [ObservableAsProperty] get; }

        public bool IsAddProjectLabelVisible { [ObservableAsProperty] get; }

        private static bool GetIsAddProjectLabelVisible(string description, string projectText)
        {
            return string.IsNullOrEmpty(projectText) && !string.IsNullOrEmpty(description);
        }

        public void SetTimeEntry(Toggl.TogglTimeEntryView item)
        {
            Description = item.Description;
            ProjectLabel.SetProject(item);
        }
    }
}