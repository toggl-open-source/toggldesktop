using System.Windows.Controls;

namespace TogglDesktop.Tutorial
{
    public class TutorialManager
    {
        private readonly MainWindow mainWindow;
        private readonly Timer timer;
        private readonly Panel tutorialPanel;
        private TutorialScreen activeScreen;

        public TutorialManager(
            MainWindow mainWindow,
            Timer timer,
            Panel tutorialPanel)
        {
            this.mainWindow = mainWindow;
            this.timer = timer;
            this.tutorialPanel = tutorialPanel;
        }

        public Timer Timer { get { return this.timer; } }

        public void QuitTutorial()
        {
            if (this.mainWindow.TryBeginInvoke(this.QuitTutorial))
                return;

            this.activateScreen(null);
        }

        public void ActivateScreen<T>()
            where T : TutorialScreen, new()
        {
            if (this.mainWindow.TryBeginInvoke(this.ActivateScreen<T>))
                return;

            this.activateScreen(new T());
        }

        private void activateScreen(TutorialScreen screen)
        {
            if (this.activeScreen != null)
            {
                this.tutorialPanel.Children.Remove(this.activeScreen);
                this.activeScreen.Dispose();
            }

            this.activeScreen = screen;

            if (screen != null)
            {
                this.tutorialPanel.Children.Add(screen);
                screen.Initialise(this);
            }
        }

    }
}