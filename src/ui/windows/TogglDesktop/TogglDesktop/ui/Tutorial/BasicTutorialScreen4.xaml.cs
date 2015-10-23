
namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen4
    {
        public BasicTutorialScreen4()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
        }

        protected override void cleanup()
        {
            Toggl.OnTimeEntryEditor -= this.onTimeEntryEditor;
            Toggl.OnStoppedTimerState -= this.onStoppedTimerState;
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            this.activateScreen<BasicTutorialScreen5>();
        }

        private void onStoppedTimerState()
        {
            this.activateScreen<BasicTutorialScreen7>();
        }


    }
}
