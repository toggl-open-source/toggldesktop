
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop.WPF
{
    public partial class PreferencesWindow
    {
        public PreferencesWindow()
        {
            this.InitializeComponent();

            Toggl.OnSettings += this.onSettings;
            Toggl.OnLogin += this.onLogin;
        }

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryInvoke(() => this.onLogin(open, userID)))
                return;

            this.recordTimelineCheckBox.IsEnabled = !open && userID != 0;
            this.recordTimelineCheckBox.IsChecked = Toggl.IsTimelineRecordingEnabled();
        }

        private void onSettings(bool open, Toggl.Settings settings)
        {
            if (this.TryInvoke(() => this.onSettings(open, settings)))
                return;

            this.updateUI(settings);

            if (open)
            {
                this.Show();
                this.Topmost = true;
            }
        }

        private void updateUI(Toggl.Settings settings)
        {
            #region general

            this.idleDetectionCheckBox.IsChecked = settings.UseIdleDetection;
            this.idleDetectionDurationTextBox.Text = settings.IdleMinutes.ToString();

            this.recordTimelineCheckBox.IsChecked = settings.RecordTimeline;
            this.onTopCheckBox.IsChecked = settings.OnTop;

            #endregion

            #region proxy

            this.useSystemProxySettingsCheckBox.IsChecked = settings.AutodetectProxy;
            this.useProxyCheckBox.IsChecked = settings.UseProxy;
            this.proxyHostTextBox.Text = settings.ProxyHost;
            this.proxyPortTextBox.Text = settings.ProxyPort.ToString();
            this.proxyUsernameTextBox.Text = settings.ProxyUsername;
            this.proxyPasswordBox.Password = settings.ProxyPassword;

            #endregion

            #region reminder

            this.remindToTrackCheckBox.IsChecked = settings.Reminder;
            this.remindToTrackIntervalTextBox.Text = settings.ReminderMinutes.ToString();
            this.reminderStartTimeTextBox.Text = settings.RemindStarts;
            this.reminderEndTimeTextBox.Text = settings.RemindEnds;

            this.remindOnMondayTextBox.IsChecked = settings.RemindMon;
            this.remindOnTuesdayTextBox.IsChecked = settings.RemindTue;
            this.remindOnWednesdayTextBox.IsChecked = settings.RemindWed;
            this.remindOnThursdayTextBox.IsChecked = settings.RemindThu;
            this.remindOnFridayTextBox.IsChecked = settings.RemindFri;
            this.remindOnSaturdayTextBox.IsChecked = settings.RemindSat;
            this.remindOnSundayTextBox.IsChecked = settings.RemindSun;

            #endregion

            //TODO: global shortcuts
        }

        private Toggl.Settings createSettingsFromUI()
        {
            var settings = new Toggl.Settings
            {
                #region general

                UseIdleDetection = isChecked(this.idleDetectionCheckBox),
                IdleMinutes = toULong(this.idleDetectionDurationTextBox.Text),

                RecordTimeline = isChecked(this.recordTimelineCheckBox),
                OnTop = isChecked(this.onTopCheckBox),

                #endregion

                #region proxy

                AutodetectProxy = isChecked(this.useSystemProxySettingsCheckBox),
                UseProxy = isChecked(this.useProxyCheckBox),
                ProxyHost = this.proxyHostTextBox.Text,
                ProxyPort = toULong(this.proxyPortTextBox.Text),
                ProxyUsername = this.proxyUsernameTextBox.Text,
                ProxyPassword = this.proxyPasswordBox.Password,

                #endregion

                #region reminder

                Reminder = isChecked(this.remindToTrackCheckBox),
                ReminderMinutes = toULong(this.remindToTrackIntervalTextBox.Text),
                RemindStarts = this.reminderStartTimeTextBox.Text,
                RemindEnds = this.reminderEndTimeTextBox.Text,

                RemindMon = isChecked(this.remindOnMondayTextBox),
                RemindTue = isChecked(this.remindOnTuesdayTextBox),
                RemindWed = isChecked(this.remindOnWednesdayTextBox),
                RemindThu = isChecked(this.remindOnThursdayTextBox),
                RemindFri = isChecked(this.remindOnFridayTextBox),
                RemindSat = isChecked(this.remindOnSaturdayTextBox),
                RemindSun = isChecked(this.remindOnSundayTextBox),

                #endregion
            };

            return settings;
        }

        private static bool isChecked(CheckBox checkBox)
        {
            return checkBox.IsChecked ?? false;
        }

        private static ulong toULong(string text)
        {
            ulong ret;
            ulong.TryParse(text, out ret);
            return ret;
        }

        private void saveButtonClicked(object sender, RoutedEventArgs e)
        {
            //TODO: global shortcuts

            var settings = this.createSettingsFromUI();

            if(Toggl.SetSettings(settings))
                this.Hide();
        }

        private void cancelButtonClicked(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void headerLeftMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }
    }
}
