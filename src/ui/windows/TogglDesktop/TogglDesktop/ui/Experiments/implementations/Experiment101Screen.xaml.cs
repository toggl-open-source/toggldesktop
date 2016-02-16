
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment101Screen
    {
        public Experiment101Screen()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();

            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
        }

        protected override void cleanup()
        {
            Toggl.OnStoppedTimerState -= this.onStoppedTimerState;
            Toggl.OnTimeEntryEditor -= this.onTimeEntryEditor;
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string f)
        {
            this.quitTutorial();
        }

        private void onStoppedTimerState()
        {
            this.quitTutorial();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();
        }
    }
}
