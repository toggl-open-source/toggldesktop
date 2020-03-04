using System;
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment99Screen
    {
        public Experiment99Screen()
        {
            this.InitializeComponent();
        }

        private IDisposable _subscription;
        protected override void initialise()
        {
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();

            _subscription = Toggl.OnStoppedTimerState.Subscribe(_ => this.quitTutorial());
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();
        }
    }
}
