using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;

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

        public MainWindow MainWindow { get { return this.mainWindow; } }
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
                this.removeScreen(this.activeScreen, screen != null);
            }

            this.activeScreen = screen;

            if (screen != null)
            {
                this.addScreen(screen);
            }
        }

        private const double screenFadeTime = 0.15;

        private void addScreen(TutorialScreen screen)
        {
            var anim = new DoubleAnimation(0, 1, TimeSpan.FromSeconds(screenFadeTime));
            screen.BeginAnimation(UIElement.OpacityProperty, anim);

            this.tutorialPanel.Children.Add(screen);
            screen.Initialise(this);
        }

        private void removeScreen(TutorialScreen screen, bool waitForNextScreenToFadeIn)
        {
            var anim = new DoubleAnimation(0, TimeSpan.FromSeconds(screenFadeTime))
            {
                BeginTime = TimeSpan.FromSeconds(
                    waitForNextScreenToFadeIn ? screenFadeTime : 0)
            };
            anim.Completed += (sender, args) =>
            {
                this.tutorialPanel.Children.Remove(screen);
            };
            screen.BeginAnimation(UIElement.OpacityProperty, anim);

            screen.IsEnabled = false;
            screen.Cleanup();
        }
    }
}