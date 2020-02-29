namespace TogglDesktop.ViewModels
{
    public class TimeEntryLabelViewModel
    {
        public TimeEntryLabelViewModel(
            string description,
            ProjectLabelViewModel projectLabel,
            string tags,
            bool isBillable,
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
            Tags = tags;
            IsBillable = isBillable;
            TagsToolTip = string.IsNullOrEmpty(Tags)
                ? null
                : Tags.Replace(Toggl.TagSeparator, " • ");
        }

        public ProjectLabelViewModel ProjectLabel { get; }

        public string Description { get; }

        public string Tags { get; }

        public string TagsToolTip { get; }

        public bool IsBillable { get; }

        public string AddDescriptionLabelText { get; }

        public bool IsAddProjectLabelVisible { get; }

        private static bool GetIsAddProjectLabelVisible(string description, string projectText)
        {
            return string.IsNullOrEmpty(projectText) && !string.IsNullOrEmpty(description);
        }
    }
}