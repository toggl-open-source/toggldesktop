using System;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.Tutorial;

namespace TogglDesktop
{
    static class KeyboardShortcuts
    {
        private static MainWindow mainWindow;

        public static void SetMainWindow(MainWindow window)
        {
            mainWindow = window;
        }

        public static void RegisterShortcuts(Window window)
        {
            window.InputBindings.AddRange(mainWindow.InputBindings);
        }

        static KeyboardShortcuts()
        {
            var bindings = new[]
            {
                new CommandBinding(New, onNew, canExecuteNew),
                new CommandBinding(NewFromPaste, onNewFromPaste, canExecuteNewFromPaste),
                new CommandBinding(Continue, onContinue, canExecuteContinue),
                new CommandBinding(Stop, onStop, canExecuteStop),
                new CommandBinding(Show, onShow, canExecuteShow),
                new CommandBinding(Hide, onHide, canExecuteHide),
                new CommandBinding(Sync, onSync, canExecuteSync),
                new CommandBinding(Reports, onReports, canExecuteReports),
                new CommandBinding(Preferences, onPreferences),
                new CommandBinding(ToggleManualMode, onToggleManualMode, canExecuteToggleManualMode),
                new CommandBinding(ClearCache, onClearCache, canExecuteClearCache),
                new CommandBinding(SendFeedback, onSendFeedback),
                new CommandBinding(About, onAbout),
                new CommandBinding(Logout, onLogout, canExecuteLogout),
                new CommandBinding(Quit, onQuit),
                new CommandBinding(EditRunning, onEditRunning, canExecuteEditRunning),
                new CommandBinding(BasicTutorial, onBasicTutorial, canExecuteBasicTutorial),
                new CommandBinding(ToggleMiniTimerVisibility, onToggleMiniTimerVisibility, canExecuteToggleMiniTimerVisibility), 
            };

            foreach(var binding in bindings)
            {
                CommandManager.RegisterClassCommandBinding(typeof(Window), binding);
            }
        }

        #region helper properties

        private static bool isTracking { get { return mainWindow.IsTracking; } }
        private static bool isInManualMode { get { return mainWindow.IsInManualMode; } }
        private static bool isLoggedIn { get { return Program.IsLoggedIn; } }
        private static bool isMiniTimerVisible { get { return mainWindow.IsMiniTimerVisible; } }

        #endregion

        #region New

        public static readonly RoutedUICommand New =
            new RoutedUICommand("", "New", typeof(Window));

        private static void canExecuteNew(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onNew(object sender, RoutedEventArgs e)
        {
            StartTimeEntry(preventOnApp: sender is MiniTimerWindow);
        }

        #endregion
        #region NewFromPaste

        public static readonly RoutedUICommand NewFromPaste =
            new RoutedUICommand("", "NewFromPaste", typeof(Window));

        private static void canExecuteNewFromPaste(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn && !isTracking;
        }

        private static void onNewFromPaste(object sender, ExecutedRoutedEventArgs e)
        {
            StartTimeEntry(description:Clipboard.GetText().Replace(Environment.NewLine, " "),
                preventOnApp: sender is MiniTimerWindow);
        }

        #endregion
        #region Continue

        public static readonly RoutedUICommand Continue =
            new RoutedUICommand("", "Continue", typeof(Window));

        private static void canExecuteContinue(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn && !isTracking;
        }

        private static void onContinue(object sender, RoutedEventArgs e)
        {
            Toggl.ContinueLatest(sender is MiniTimerWindow);
        }

        #endregion
        #region Stop

        public static readonly RoutedUICommand Stop =
            new RoutedUICommand("", "Stop", typeof(Window));

        private static void canExecuteStop(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn && isTracking;
        }

        private static void onStop(object sender, RoutedEventArgs e)
        {
            Toggl.Stop(sender is MiniTimerWindow);
        }

        #endregion
        #region Show

        public static readonly RoutedUICommand Show =
            new RoutedUICommand("", "Show", typeof(Window));

        private static void canExecuteShow(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = mainWindow.CanBeShown;
        }

        private static void onShow(object sender, RoutedEventArgs e)
        {
            mainWindow.ShowOnTop();
        }

        #endregion
        #region Hide

        public static readonly RoutedUICommand Hide =
            new RoutedUICommand("", "Hide", typeof(Window));

        private static void canExecuteHide(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !mainWindow.CanBeShown;
        }

        private static void onHide(object sender, ExecutedRoutedEventArgs e)
        {
            mainWindow.MinimizeToTray();
        }

        #endregion
        #region Sync

        public static readonly RoutedUICommand Sync =
            new RoutedUICommand("", "Sync", typeof(Window));

        private static void canExecuteSync(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onSync(object sender, RoutedEventArgs e)
        {
            Toggl.Sync();
        }

        #endregion
        #region Reports

        public static readonly RoutedUICommand Reports =
            new RoutedUICommand("", "Reports", typeof(Window));

        private static void canExecuteReports(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onReports(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser();
        }

        #endregion
        #region Preferences

        public static readonly RoutedUICommand Preferences =
            new RoutedUICommand("", "Preferences", typeof(Window));

        private static void onPreferences(object sender, RoutedEventArgs e)
        {
            Toggl.EditPreferences();
        }

        #endregion
        #region ToggleManualMode

        public static readonly RoutedUICommand ToggleManualMode =
            new RoutedUICommand("", "ToggleManualMode", typeof(Window));

        private static void canExecuteToggleManualMode(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn && (isInManualMode || !isTracking);
        }

        private static void onToggleManualMode(object sender, RoutedEventArgs e)
        {
            mainWindow.SetManualMode(!isInManualMode);
        }

        #endregion
        #region ClearCache

        public static readonly RoutedUICommand ClearCache =
            new RoutedUICommand("", "ClearCache", typeof(Window));

        private static void canExecuteClearCache(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onClearCache(object sender, RoutedEventArgs e)
        {
            var result = MessageBox.Show(mainWindow,
                "This will remove your Toggl user data from this PC and log you out of the Toggl Desktop app. " +
                "Any unsynced data will be lost.\n\nDo you want to continue?", "Clear Cache",
                MessageBoxButton.OKCancel, "CLEAR CACHE");

            if (result == MessageBoxResult.OK)
            {
                Toggl.ClearCache();
            }
        }

        #endregion
        #region SendFeedback

        public static readonly RoutedUICommand SendFeedback =
            new RoutedUICommand("", "SendFeedback", typeof(Window));

        private static void onSendFeedback(object sender, RoutedEventArgs e)
        {
            mainWindow.GetWindow<FeedbackWindow>().Show();
            mainWindow.GetWindow<FeedbackWindow>().Activate();
        }

        #endregion
        #region About

        public static readonly RoutedUICommand About =
            new RoutedUICommand("", "About", typeof(Window));

        private static void onAbout(object sender, RoutedEventArgs e)
        {
            mainWindow.GetWindow<AboutWindow>().Show();
            mainWindow.GetWindow<AboutWindow>().Activate();
        }

        #endregion
        #region Logout

        public static readonly RoutedUICommand Logout =
            new RoutedUICommand("", "Logout", typeof(Window));

        private static void canExecuteLogout(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onLogout(object sender, RoutedEventArgs e)
        {
            Toggl.Logout();
        }

        #endregion
        #region Quit

        public static readonly RoutedUICommand Quit =
            new RoutedUICommand("", "Quit", typeof(Window));

        private static void onQuit(object sender, RoutedEventArgs e)
        {
            if (mainWindow.IsVisible)
            {
                mainWindow.Close();
            }
            else
            {
                Environment.Exit(0);
            }
        }

        #endregion
        #region EditRunning

        public static readonly RoutedUICommand EditRunning =
            new RoutedUICommand("", "EditRunning", typeof(Window));

        private static void canExecuteEditRunning(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn && (isTracking || isInManualMode);
        }

        private static void onEditRunning(object sender, RoutedEventArgs e)
        {
            if (isInManualMode)
            {
                StartTimeEntry();
            }
            else
            {
                Toggl.Edit(null, true, null);
            }
        }

        #endregion
        #region BasicTutorial

        public static readonly RoutedUICommand BasicTutorial =
            new RoutedUICommand("", "BasicTutorial", typeof(Window));

        private static void canExecuteBasicTutorial(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onBasicTutorial(object sender, ExecutedRoutedEventArgs e)
        {
            mainWindow.TutorialManager.ActivateScreen<BasicTutorialScreen1>();
        }

        #endregion
        #region ToggleMiniTimerVisibility

        public static readonly RoutedUICommand ToggleMiniTimerVisibility =
            new RoutedUICommand("", "ToggleManualMode", typeof(Window));

        private static void canExecuteToggleMiniTimerVisibility(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = isLoggedIn;
        }

        private static void onToggleMiniTimerVisibility(object sender, RoutedEventArgs e)
        {
            mainWindow.SetMiniTimerVisible(!isMiniTimerVisible);
        }

        #endregion

        #region implementations

        public static void StartTimeEntry(bool continueIfNotInManualMode = false,
            string description = "", bool preventOnApp = false)
        {
            if (isInManualMode)
            {
                var guid = Toggl.Start(description, "0", 0, 0, "", "");
                Toggl.Edit(guid, false, Toggl.Duration);
            }
            else
            {
                if (continueIfNotInManualMode)
                {
                    Toggl.ContinueLatest(preventOnApp);
                }
                else
                {
                    Toggl.Start(description, "", 0, 0, "", "", preventOnApp);
                }
            }
        }
        #endregion
    }
}