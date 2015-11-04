
using System.Windows;

namespace TogglDesktop.Tutorial
{
    public partial class NewUserWelcomeScreen
    {
        public NewUserWelcomeScreen()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
        }

        protected override void cleanup()
        {
        }

        private void startTutorialButtonClick(object sender, RoutedEventArgs e)
        {
            this.activateScreen<BasicTutorialScreen1>();
        }

        private void skipTutorialButtonClicked(object sender, RoutedEventArgs e)
        {
            this.quitTutorial();
        }
    }
}
