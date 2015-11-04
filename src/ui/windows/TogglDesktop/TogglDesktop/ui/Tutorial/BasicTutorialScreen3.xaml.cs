
namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen3
    {
        public BasicTutorialScreen3()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnRunningTimerState += this.onRunningTimerState;
        }

        protected override void cleanup()
        {
            Toggl.OnRunningTimerState -= this.onRunningTimerState;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<BasicTutorialScreen4>();
        }

    }
}
