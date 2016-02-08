
namespace TogglDesktop.Experiments
{
    public partial class Experiment99Screen
    {
        public Experiment99Screen()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();

            Toggl.OnRunningTimerState += this.onRunningTimerState;
        }

        protected override void cleanup()
        {
            Toggl.OnRunningTimerState -= this.onRunningTimerState;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.quitTutorial();
        }
    }
}
