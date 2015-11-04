
using System.Windows;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen7
    {
        public BasicTutorialScreen7()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnTimeEntryEditor += this.onTimerEntryEditor;
        }

        protected override void cleanup()
        {
            Toggl.OnRunningTimerState -= this.onRunningTimerState;
            Toggl.OnTimeEntryEditor -= this.onTimerEntryEditor;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.quitTutorial();
        }
        private void onTimerEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            this.quitTutorial();
        }
    }
}
