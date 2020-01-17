using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop
{
    public partial class TimeEntryLabel : UserControl
    {
        public TimeEntryLabel()
        {
            InitializeComponent();
        }

        public event MouseButtonEventHandler DescriptionLabelMouseDown;
        public event MouseButtonEventHandler ProjectLabelMouseDown;

        private double _verticalPadding;
        public double VerticalPadding
        {
            get => _verticalPadding;
            set
            {
                _verticalPadding = value;
                descriptionLabel.Margin = new Thickness(0, 0, 0, _verticalPadding);
                noDescriptionLabel.Margin = new Thickness(0, 0, 0, _verticalPadding);
                projectLabel.Margin = new Thickness(0, _verticalPadding, 0, 0);
            }
        }

        private bool _compactDescription;

        public bool CompactDescription
        {
            get => _compactDescription;
            set
            {
                _compactDescription = value;
                descriptionLabel.LineHeight = 16;
                descriptionLabel.LineStackingStrategy = LineStackingStrategy.BlockLineHeight;
                noDescriptionLabel.LineHeight = 16;
                noDescriptionLabel.LineStackingStrategy = LineStackingStrategy.BlockLineHeight;
            }
        }

        public void SetTimeEntry(Toggl.TogglTimeEntryView item)
        {
            this.descriptionLabel.Text = item.Description;
            this.noDescriptionLabel.ShowOnlyIf(descriptionLabel.Text.IsNullOrEmpty());

            this.projectLabel.SetProject(item);

            if (item.Description.IsNullOrEmpty() && item.ProjectLabel.IsNullOrEmpty())
            {
                this.projectLabel.Visibility = Visibility.Collapsed;
                this.noDescriptionLabel.Text = "+ Add details";
            }
            else
            {
                this.projectLabel.Visibility = Visibility.Visible;
                this.noDescriptionLabel.Text = "+ Add description";
            }
        }

        public void ResetUIState(bool running)
        {
            this.projectLabel.ShowOnlyIf(running);
            this.descriptionLabel.ShowOnlyIf(running);
            this.noDescriptionLabel.Text = "+ Add description";
            this.noDescriptionLabel.ShowOnlyIf(running && descriptionLabel.Text.IsNullOrEmpty());
        }

        public void ClearProject()
        {
            this.projectLabel.Clear();
        }

        private void onDescriptionLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            DescriptionLabelMouseDown?.Invoke(sender, e);
        }

        private void onProjectLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            ProjectLabelMouseDown?.Invoke(sender, e);
        }
    }
}