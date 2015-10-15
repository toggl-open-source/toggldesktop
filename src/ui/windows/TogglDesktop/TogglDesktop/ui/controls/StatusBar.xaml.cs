using System;
using System.Windows;

namespace TogglDesktop
{
    public partial class StatusBar
    {
        private Toggl.OnlineState onlineState;

        public StatusBar()
        {
            this.InitializeComponent();

            Toggl.OnOnlineState += this.onOnlineState;
            Toggl.OnLogin += this.onLogin;
        }

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open)
            {
                this.Hide();
            }
        }

        private void onOnlineState(Toggl.OnlineState state)
        {
            if (this.TryBeginInvoke(this.onOnlineState, state))
                return;

            this.onlineState = state;
            this.update();
        }

        private void update()
        {
            if (!Program.IsLoggedIn)
            {
                this.Hide();
                return;
            }

            switch (this.onlineState)
            {
                case Toggl.OnlineState.Online:
                {
                    this.Hide();
                    return;
                }
                case Toggl.OnlineState.NoNetwork:
                {
                    this.statusText.Text = "Offline, no network";
                    break;
                }
                case Toggl.OnlineState.BackendDown:
                {
                    this.statusText.Text = "Offline, Toggl not responding";
                    break;
                }
                default:
                    throw new ArgumentOutOfRangeException();
            }

            this.Visibility = Visibility.Visible;
        }

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }
    }
}
