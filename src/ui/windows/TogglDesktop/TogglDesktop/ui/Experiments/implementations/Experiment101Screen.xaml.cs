using System;
using System.Reactive.Disposables;
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment101Screen
    {
        private IDisposable _subscription;
        public Experiment101Screen()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();

            _subscription = new CompositeDisposable(
                Toggl.OnStoppedTimerState.Subscribe(_ => this.quitTutorial()),
                Toggl.OnTimeEntryEditor.Subscribe(_ => this.quitTutorial()));
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
