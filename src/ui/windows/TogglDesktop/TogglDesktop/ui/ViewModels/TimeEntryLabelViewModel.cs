using System.Reactive.Linq;
using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class TimeEntryLabelViewModel : ReactiveObject
    {
        public TimeEntryLabelViewModel(ProjectLabelViewModel projectLabel)
        {
            ProjectLabel = projectLabel;
            _addDescriptionLabelText = this.WhenAnyValue(x => x.ProjectLabel.ProjectName)
                .Select(projectName => string.IsNullOrEmpty(projectName) ? "+ Add details" : "+ Add description")
                .ToProperty(this, nameof(AddDescriptionLabelText));
            _isAddProjectLabelVisible = this.WhenAnyValue(x => x.Description, x => x.ProjectLabel.ProjectName)
                .Select(((string description, string projectName) tuple) => GetIsAddProjectLabelVisible(tuple.description, tuple.projectName))
                .ToProperty(this, nameof(IsAddProjectLabelVisible));
        }
        private ProjectLabelViewModel _projectLabel;
        public ProjectLabelViewModel ProjectLabel
        {
            get => _projectLabel;
            private set => this.RaiseAndSetIfChanged(ref _projectLabel, value);
        }

        private string _description;
        public string Description
        {
            get => _description;
            set => this.RaiseAndSetIfChanged(ref _description, value);
        }

        private readonly ObservableAsPropertyHelper<string> _addDescriptionLabelText;
        public string AddDescriptionLabelText => _addDescriptionLabelText.Value;

        private readonly ObservableAsPropertyHelper<bool> _isAddProjectLabelVisible;
        public bool IsAddProjectLabelVisible => _isAddProjectLabelVisible.Value;

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