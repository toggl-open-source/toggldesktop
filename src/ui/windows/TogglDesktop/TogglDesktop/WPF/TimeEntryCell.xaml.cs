using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for TimeEntryCell.xaml
    /// </summary>
    public partial class TimeEntryCell
    {
        public string GUID { get; set; }

        public Color EntryBackColor
        {
            get { return (Color)this.GetValue(EntryBackColorProperty); }
            set { this.SetValue(EntryBackColorProperty, value); }
        }
        public static readonly DependencyProperty EntryBackColorProperty = DependencyProperty
            .Register("EntryBackColor", typeof(Color), typeof(TimeEntryCell), new FrameworkPropertyMetadata(Color.FromArgb(255, 0, 0, 0)));

        private readonly ToolTip descriptionToolTip = new ToolTip();
        private readonly ToolTip taskProjectClientToolTip = new ToolTip();
        private readonly ToolTip durationToolTip = new ToolTip();
        private readonly ToolTip tagsToolTip = new ToolTip();

        public TimeEntryCell()
        {
            InitializeComponent();
            DataContext = this;

            this.labelDescription.ToolTip = this.descriptionToolTip;
            this.labelTask.ToolTip = this.labelProject.ToolTip = this.labelClient.ToolTip = this.taskProjectClientToolTip;
            this.labelDuration.ToolTip = this.durationToolTip;
            this.tagsIcon.ToolTip = this.tagsToolTip;
        }

        public void Display(Toggl.TimeEntry item)
        {
            GUID = item.GUID;

            labelDescription.Text = item.Description == "" ? "(no description)" : item.Description;
            
            var projectColorBrush = getProjectColorBrush(ref item);

            projectColor.Fill = projectColorBrush;
            labelProject.Foreground = projectColorBrush;
            labelProject.Text = (item.ClientLabel.Length > 0) ? "• " + item.ProjectLabel : item.ProjectLabel;
            setOptionalTextBlockText(labelClient, item.ClientLabel);
            setOptionalTextBlockText(labelTask, item.TaskLabel);
            labelDuration.Text = item.Duration;
            showOnlyIf(billabeIcon, item.Billable);
            showOnlyIf(tagsIcon, !string.IsNullOrEmpty(item.Tags));

            showOnlyIf(dayHeader, item.IsHeader);

            if (item.IsHeader)
            {
                labelFormattedDate.Text = item.DateHeader;
                labelDateDuration.Text = item.DateDuration;
            }

            updateToolTips(item);

        }

        private void updateToolTips(Toggl.TimeEntry item)
        {
            this.descriptionToolTip.Content = item.Description;
            this.taskProjectClientToolTip.Content = item.ProjectAndTaskLabel;

            if (item.DurOnly)
            {
                this.labelDuration.ToolTip = null;
            }
            else
            {
                this.labelDuration.ToolTip = this.durationToolTip;
                durationToolTip.Content = item.StartTimeString + " - " + item.EndTimeString;
            }

            if (tagsIcon.Visibility == Visibility.Visible)
            {
                tagsToolTip.Content = item.Tags.Replace(Toggl.TagSeparator, ", ");
            }
        }

        #region display helpers

        private static void showOnlyIf(FrameworkElement element, bool condition)
        {
            element.Visibility = condition ? Visibility.Visible : Visibility.Collapsed;
        }

        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            showOnlyIf(textBlock, !string.IsNullOrEmpty(text));
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TimeEntry item)
        {
            var projectColourString = item.Color != "" ? item.Color : "#999999";
            var projectColor = (Color)ColorConverter.ConvertFromString(projectColourString);
            var projectColorBrush = new SolidColorBrush(projectColor);
            return projectColorBrush;
        }

        #endregion

        #region open edit window event handlers

        private void labelDuration_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Duration);
            e.Handled = true;
        }

        private void labelDescription_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Description);
            e.Handled = true;
        }

        private void labelProject_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
            e.Handled = true;
        }

        private void entry_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, "");
            e.Handled = true;
        }

        #endregion

        private void buttonContinue_Click(object sender, RoutedEventArgs e)
        {
            Toggl.Continue(GUID);
        }


    }
}
