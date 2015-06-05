using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for TimeEntryCell.xaml
    /// </summary>
    public partial class TimeEntryCell : UserControl
    {
        public string GUID;
        public bool header = false;
        private TimeEntryListViewController list;

        public TimeEntryCell(TimeEntryListViewController listContainer)
        {
            list = listContainer;
            InitializeComponent();

            //this.defaultColor = this.panel.BackColor;
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

            header = item.IsHeader;
            showOnlyIf(dayHeader, header);

            if (header)
            {
                labelFormattedDate.Text = item.DateHeader;
                labelDateDuration.Text = item.DateDuration;
            }
            //toolTip.SetToolTip(labelDescription, item.Description);
            //toolTip.SetToolTip(labelTask, item.ProjectAndTaskLabel);
            //toolTip.SetToolTip(labelProject, item.ProjectAndTaskLabel);
            //toolTip.SetToolTip(labelClient, item.ProjectAndTaskLabel);
            //if (!item.DurOnly)
            //{
            //    toolTip.SetToolTip(labelDuration, item.StartTimeString + " - " + item.EndTimeString);
            //}
            //if (labelTag.Visible)
            //{
            //    toolTip.SetToolTip(labelTag, item.Tags.Replace(Toggl.TagSeparator, ", "));
            //}
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
        }

        private void labelDescription_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Description);
        }

        private void labelProject_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
        }

        private void entry_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, "");
        }

        #endregion

        private void buttonContinue_Click(object sender, RoutedEventArgs e)
        {
            Toggl.Continue(GUID);
        }


    }
}
