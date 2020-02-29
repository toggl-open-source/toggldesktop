namespace TogglDesktop.ViewModels
{
    public class TimeEntryLabelViewModel
    {
        public TimeEntryLabelViewModel(
            string description,
            ProjectLabelViewModel projectLabel,
            bool showAddDetailsLabels = true)
        {
            Description = description;
            ProjectLabel = projectLabel;
            AddDescriptionLabelText =
                showAddDetailsLabels
                    ? (string.IsNullOrEmpty(ProjectLabel.ProjectName)
                        ? "+ Add details"
                        : "+ Add description")
                    : "(no description)";
            IsAddProjectLabelVisible = showAddDetailsLabels &&
                                       GetIsAddProjectLabelVisible(Description, ProjectLabel.ProjectName);
        }

        public ProjectLabelViewModel ProjectLabel { get; }

        public string Description { get; }

        public string AddDescriptionLabelText { get; }

        public bool IsAddProjectLabelVisible { get; }

        private static bool GetIsAddProjectLabelVisible(string description, string projectText)
        {
            return string.IsNullOrEmpty(projectText) && !string.IsNullOrEmpty(description);
        }
    }
}