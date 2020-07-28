using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Animation;

namespace TogglDesktop
{
    public partial class SyncingIndicator
    {
        private readonly Storyboard _spinnerAnimation;

        private long _unsyncedItems;
        private Toggl.SyncState _syncState;
        private bool _spinnerIsAnimating;

        public SyncingIndicator()
        {
            this.InitializeComponent();

            this._spinnerAnimation = (Storyboard)this.Resources["RotateSpinner"];

            Toggl.OnDisplaySyncState += this.onDisplaySyncState;
            Toggl.OnDisplayUnsyncedItems += this.onDisplayUnsyncedItems;
            Toggl.OnLogin += this.onLogin;
            Toggl.OnManualSync += this.onManualSync;
        }

        private bool hasSomethingToShow
        {
            get { return Program.IsLoggedIn && (this._unsyncedItems != 0 || this._syncState != Toggl.SyncState.Idle); }
        }

        #region toggl events

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open)
            {
                this.Hide();
            }
        }

        private void onDisplayUnsyncedItems(long count)
        {
            if (this.TryBeginInvoke(this.onDisplayUnsyncedItems, count))
                return;

            this._unsyncedItems = count;
            this.update();
        }

        private void onDisplaySyncState(Toggl.SyncState state)
        {
            if (this.TryBeginInvoke(this.onDisplaySyncState, state))
                return;

            this._syncState = state;
            this.update();
        }

        private void onManualSync()
        {
            if (this.TryBeginInvoke(this.onManualSync))
                return;

            this._syncState = Toggl.SyncState.Syncing;
            this.show();
        }

        #endregion

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }

        private void update()
        {
            if (!this.hasSomethingToShow)
            {
                this.stopSpinnerAnimation();
                this.Hide();
                return;
            }

            if (this.IsVisible)
            {
                this.show();
            }
            else
            {
                this.tryShowingDelayed();
            }
        }

        private void show()
        {
            this.unsyncedCount.Text = this._unsyncedItems == 0
                ? "" : this._unsyncedItems.ToString();

            switch (this._syncState)
            {
                case Toggl.SyncState.Idle:
                    {
                        this.ToolTip = string.Format("{0} unsynced time entries. Click to Sync.", this._syncState);
                        this.stopSpinnerAnimation();
                        break;
                    }

                case Toggl.SyncState.Syncing:
                    {
                        this.ToolTip = "Syncing...";
                        this.startSpinnerAnimation();
                        break;
                    }

                default:
                    throw new ArgumentOutOfRangeException();
            }

            this.Visibility = Visibility.Visible;
        }

        private async void tryShowingDelayed()
        {
            await Task.Delay(TimeSpan.FromSeconds(3));

            if (this.IsVisible)
                return;

            if (this.hasSomethingToShow)
            {
                this.show();
            }
        }

        private void stopSpinnerAnimation()
        {
            if (!this._spinnerIsAnimating)
                return;

            this._spinnerAnimation.Stop();
            this._spinnerIsAnimating = false;
            this.syncButton.IsEnabled = true;
        }

        private void startSpinnerAnimation()
        {
            if (this._spinnerIsAnimating)
                return;

            this._spinnerAnimation.Begin();
            this._spinnerIsAnimating = true;
            this.syncButton.IsEnabled = false;
        }

        private void onSyncButtonClick(object sender, RoutedEventArgs e)
        {
            Toggl.Sync();
        }
    }
}
