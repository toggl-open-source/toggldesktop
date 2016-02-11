
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment98Screen2
    {
        public Experiment98Screen2()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;

            Toggl.SendObmAction(98, "seen_2");
        }

        protected override void cleanup()
        {
            Toggl.OnStoppedTimerState -= this.onStoppedTimerState;
            Toggl.OnTimeEntryEditor -= this.onTimeEntryEditor;
        }

        private void onStoppedTimerState()
        {
            this.quitTutorial();
        }
        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focused_field_name)
        {
            this.quitTutorial();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();
        }
    }
}
