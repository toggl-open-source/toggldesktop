using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop
{
    public partial class TimeEntryInfoPanel : UserControl
    {
        public TimeEntryInfoPanel()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty DurationTextBlockStyleProperty = DependencyProperty.Register(
            "DurationTextBlockStyle", typeof(Style), typeof(TimeEntryInfoPanel), new PropertyMetadata(default(Style)));

        public Style DurationTextBlockStyle
        {
            get { return (Style) GetValue(DurationTextBlockStyleProperty); }
            set { SetValue(DurationTextBlockStyleProperty, value); }
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

        public void SetTimeEntry(Toggl.TogglTimeEntryView item)
        {
            this.billableIcon.ShowOnlyIf(item.Billable);
            this.tagsIcon.ShowOnlyIf(!string.IsNullOrEmpty(item.Tags));
            this.durationLabel.Text = Toggl.FormatDurationInSecondsHHMMSS(item.DurationInSeconds);
            this.durationLabelPanel.ToolTip =
                item.Ended > item.Started
                ? item.StartTimeString + " - " + item.EndTimeString
                : "started at " + item.StartTimeString;
            this.tagsIcon.ToolTip =
                string.IsNullOrEmpty(item.Tags)
                ? null
                : item.Tags.Replace(Toggl.TagSeparator, " • ");
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