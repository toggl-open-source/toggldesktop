using System;
using System.Reactive.Disposables;
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment98Screen2
    {
        private IDisposable _subscription;
        public Experiment98Screen2()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            _subscription = new CompositeDisposable(
                Toggl.OnStoppedTimerState.Subscribe(_ => this.quitTutorial()),
                Toggl.OnTimeEntryEditor.Subscribe(_ => this.quitTutorial()));

            Toggl.SendObmAction(98, "seen_2");
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
