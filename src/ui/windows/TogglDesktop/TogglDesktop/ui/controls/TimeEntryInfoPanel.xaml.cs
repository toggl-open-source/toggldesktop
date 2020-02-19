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

        public void UpdateBillableAndTags(bool billable, string tags)
        {
            this.billableIcon.ShowOnlyIf(billable);
            this.tagsIcon.ShowOnlyIf(!string.IsNullOrEmpty(tags));
            this.tagsIcon.Tag = tags;
            this.tagsIcon.ToolTip =
                string.IsNullOrEmpty(tags)
                    ? null
                    : tags.Replace(Toggl.TagSeparator, " • ");
        }

        public bool IsBillable => billableIcon.IsVisible;
        public string TagsString => tagsIcon.Tag?.ToString() ?? string.Empty;

        public void ResetUIState(bool running)
        {
            this.ShowOnlyIf(running);
            this.UpdateBillableAndTags(false, string.Empty);
            this.durationLabelPanel.ToolTip = null;
            this.durationLabel.Text = "00:00:00";
        }

        public void SetTimeEntry(Toggl.TogglTimeEntryView item)
        {
            this.UpdateBillableAndTags(item.Billable, item.Tags);
            this.durationLabel.Text =
                item.Ended > item.Started
                ? item.Duration
                : Toggl.FormatDurationInSecondsHHMMSS(item.DurationInSeconds);
            this.durationLabelPanel.ToolTip =
                item.Ended > item.Started
                ? item.StartTimeString + " - " + item.EndTimeString
                : "started at " + item.StartTimeString;
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