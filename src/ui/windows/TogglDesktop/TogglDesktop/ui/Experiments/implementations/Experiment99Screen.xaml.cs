
using System.Windows;

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

            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
        }

        protected override void cleanup()
        {
            Toggl.OnStoppedTimerState -= this.onStoppedTimerState;
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
