using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace TogglDesktop
{
    public partial class IdleNotificationWindow
    {
        private string guid;
        private long started;

        public event EventHandler AddedIdleTimeAsNewEntry;

        public IdleNotificationWindow()
        {
            this.InitializeComponent();
            Toggl.OnIdleNotification += this.onIdleNotification;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
        }

        private void onIdleNotification(string guid, string since, string duration, long started, string description)
        {
            if (this.TryBeginInvoke(this.onIdleNotification, guid, since, duration, started, description))
                return;

            this.guid = guid;
            this.started = started;

            this.timeText.Text = since;
            this.durationText.Text = duration;
            this.descriptionText.Text = description;

            this.Show();
            this.Topmost = true;
            this.Activate();
        }

        protected override void OnDeactivated(EventArgs e)
        {
            this.Topmost = true;
            this.Activate();
        }

        private void onStoppedTimerState()
        {
            if (this.TryBeginInvoke(this.onStoppedTimerState))
                return;

            if (this.IsVisible)
            {
                this.Hide();
            }
        }

        private void windowKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                this.Hide();
                e.Handled = true;
            }
        }

        private void onKeepTimeClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void onDiscardTimeClick(object sender, RoutedEventArgs e)
        {
            Toggl.DiscardTimeAt(this.guid, this.started, false);
            this.Hide();
        }

        private void onAddAsNewClick(object sender, RoutedEventArgs e)
        {
            Toggl.DiscardTimeAt(this.guid, this.started, true);
            this.Hide();
            if (this.AddedIdleTimeAsNewEntry != null)
                this.AddedIdleTimeAsNewEntry(this, EventArgs.Empty);
        }

        private void onDiscardContinueClick(object sender, RoutedEventArgs e)
        {
            Toggl.DiscardTimeAndContinue(this.guid, this.started, false);
            this.Hide();
        }
    }
}
