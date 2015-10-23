
namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen2
    {
        public BasicTutorialScreen2()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            this.tutorialManager.Timer.DescriptionTextBoxTextChanged += this.onDescriptionTextChanged;
        }

        protected override void cleanup()
        {
            Toggl.OnRunningTimerState -= this.onRunningTimerState;
            this.tutorialManager.Timer.DescriptionTextBoxTextChanged -= this.onDescriptionTextChanged;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.tutorialManager.ActivateScreen<BasicTutorialScreen4>();
        }

        private void onDescriptionTextChanged(object sender, string text)
        {
            if (string.IsNullOrWhiteSpace(text))
                return;

            this.activateScreen<BasicTutorialScreen3>();
        }

    }
}
