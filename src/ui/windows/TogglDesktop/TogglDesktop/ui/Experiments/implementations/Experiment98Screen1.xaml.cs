
using System;
using System.Reactive.Linq;
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment98Screen1
    {
        public Experiment98Screen1()
        {
            this.InitializeComponent();
        }

        private IDisposable _subscription;
        protected override void initialise()
        {
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();

            _subscription = Toggl.OnRunningTimerState.Subscribe(this.onRunningTimerState);

            Toggl.SendObmAction(98, "seen_1");
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<Experiment98Screen2>();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();

            Toggl.OnRunningTimerState.Take(1).Subscribe(this.openSecondScreenDelayed);
        }

        private void openSecondScreenDelayed(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<Experiment98Screen2>();
        }
    }
}
