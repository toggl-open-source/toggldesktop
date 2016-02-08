
using System.Windows;

namespace TogglDesktop.Experiments
{
    public partial class Experiment87Screen
    {
        public Experiment87Screen()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            this.fillDummies();

            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();
        }

        private void fillDummies()
        {
            this.dayHeaderDummy.DisplayDummy("Today");
            this.cellDummy0.Display(new Toggl.TogglTimeEntryView
            {
                Description = "Meeting",
                ProjectLabel = "Internal",
                Color = "#6ac4fc",
            }, null);
            this.cellDummy1.Display(new Toggl.TogglTimeEntryView
            {
                Description = "Research for presentation",
                ProjectLabel = "Consulting",
                ClientLabel = "Toy Factory",
                Color = "#d086ac",
            }, null);
            this.cellDummy2.Display(new Toggl.TogglTimeEntryView
            {
                Description = "Code review",
                ProjectLabel = "Frontend",
                ClientLabel = "Toy Factory",
                Color = "#69c600",
            }, null);
        }

        protected override void cleanup()
        {
        }

        private void nextButtonClick(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();
        }
    }
}
