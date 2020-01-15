using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop
{
    public partial class RunningEntryInfoPanel : UserControl
    {
        public RunningEntryInfoPanel()
        {
            InitializeComponent();
        }

        public event MouseButtonEventHandler DurationLabelMouseDown;

        public void OnConfirmCompletion(Toggl.TogglAutocompleteView item)
        {
            this.billableIcon.ShowOnlyIf(item.Billable);
            this.tagsIcon.ShowOnlyIf(!string.IsNullOrEmpty(item.Tags));
            this.tagsIcon.Tag = item.Tags;
        }

        public bool IsBillable => billableIcon.IsVisible;
        public string TagsString => tagsIcon.Tag?.ToString() ?? string.Empty;

        public void ResetUIState(bool running)
        {
            this.ShowOnlyIf(running);
            this.billableIcon.Visibility = Visibility.Collapsed;
            this.tagsIcon.Visibility = Visibility.Collapsed;
            this.tagsIcon.Tag = "";
            this.durationLabelPanel.ToolTip = null;
            this.durationLabel.Text = "00:00:00";
        }

        public void SetUIToRunningState(Toggl.TogglTimeEntryView item)
        {
            this.durationLabelPanel.ToolTip = "started at " + item.StartTimeString;
            this.billableIcon.ShowOnlyIf(item.Billable);
            this.tagsIcon.ShowOnlyIf(!string.IsNullOrEmpty(item.Tags));
            this.durationLabel.Text = Toggl.FormatDurationInSecondsHHMMSS(item.DurationInSeconds);
        }

        public void SetDurationLabel(string s)
        {
            this.durationLabel.Text = s;
        }

        private void onTimeLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            DurationLabelMouseDown?.Invoke(sender, e);
        }
    }
}