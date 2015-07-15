using System;
using System.Collections.Generic;
using System.Windows;

namespace TogglDesktop.WPF
{
    public partial class TimerEditViewController
    {
        public TimerEditViewController()
        {
            this.InitializeComponent();

            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
        }

        private void onStoppedTimerState()
        {
        }

        private void onRunningTimerState(Toggl.TimeEntry te)
        {
        }

        private void onMiniTimerAutocomplete(List<Toggl.AutocompleteItem> list)
        {
        }

        private void startStopButtonOnClick(object sender, RoutedEventArgs e)
        {
        }
    }
}
