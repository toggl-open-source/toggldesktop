
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment98Screen1
    {
        public Experiment98Screen1()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();

            Toggl.OnRunningTimerState += this.onRunningTimerState;

            Toggl.SendObmAction(98, "seen_1");
        }

        protected override void cleanup()
        {
            Toggl.OnRunningTimerState -= this.onRunningTimerState;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<Experiment98Screen2>();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();

            Toggl.OnRunningTimerState += this.openSecondScreenDelayed;
        }

        private void openSecondScreenDelayed(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<Experiment98Screen2>();

            Toggl.OnRunningTimerState -= this.openSecondScreenDelayed;
        }
    }
}
