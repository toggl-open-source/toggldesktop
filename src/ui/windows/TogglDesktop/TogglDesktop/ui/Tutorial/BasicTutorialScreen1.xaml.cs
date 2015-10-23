
using System.Windows;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen1
    {
        public BasicTutorialScreen1()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.Stop();
            Toggl.SetManualMode(false);
            Toggl.ViewTimeEntryList();
        }

        protected override void cleanup()
        {
        }

        private void nextButtonClick(object sender, RoutedEventArgs e)
        {
            this.activateScreen<BasicTutorialScreen2>();
        }
    }
}
