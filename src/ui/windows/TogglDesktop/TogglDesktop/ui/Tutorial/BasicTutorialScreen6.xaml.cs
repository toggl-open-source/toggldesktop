
namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen6
    {
        public BasicTutorialScreen6()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
        }

        protected override void cleanup()
        {
            Toggl.OnStoppedTimerState -= this.onStoppedTimerState;
        }

        private void onStoppedTimerState()
        {
            this.activateScreen<BasicTutorialScreen7>();
        }
    }
}
