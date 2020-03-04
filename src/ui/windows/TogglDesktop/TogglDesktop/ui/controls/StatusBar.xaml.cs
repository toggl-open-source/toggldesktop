using System;
using System.Reactive.Linq;
using System.Windows;

namespace TogglDesktop
{
    public partial class StatusBar
    {
        public StatusBar()
        {
            this.InitializeComponent();

            Toggl.OnOnlineState
                .CombineLatest(Toggl.OnLogin, GetState)
                .ObserveOnDispatcher()
                .Subscribe(onOnlineState);
        }

        private static Toggl.OnlineState GetState(Toggl.OnlineState onlineState, (bool open, ulong userId) x)
        {
            return x.userId == 0 ? Toggl.OnlineState.Online : onlineState;
        }

        private void onOnlineState(Toggl.OnlineState state)
        {
            switch (state)
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
