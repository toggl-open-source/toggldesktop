using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Threading;
using TogglDesktop.Diagnostics;
using MenuItem = System.Windows.Controls.MenuItem;
using MouseEventArgs = System.Windows.Input.MouseEventArgs;
using UserControl = System.Windows.Controls.UserControl;

namespace TogglDesktop
{
    public partial class MainWindow
    {
        #region fields

        private readonly DispatcherTimer idleDetectionTimer =
            new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) };
        private readonly KeyboardHook startHook = new KeyboardHook();
        private readonly KeyboardHook showHook = new KeyboardHook();

        private readonly WindowInteropHelper interopHelper;
        private readonly UserControl[] views;
        private Window[] childWindows;

        private AboutWindow aboutWindow;
        private FeedbackWindow feedbackWindow;
        private EditViewPopup editPopup;
        private IdleNotificationWindow idleNotificationWindow;
        private SyncingIndicator syncingIndicator;

        private UserControl activeView;
        private bool isInManualMode;
        private bool isTracking;
        private bool isResizingWithHandle;
        private bool closing;
        private string trackingTitle;

        #endregion

        public MainWindow()
        {
            this.DataContext = this;
            this.InitializeComponent();

            this.interopHelper = new WindowInteropHelper(this);

            this.views = new UserControl[] {this.loginView, this.timerEntryListView};

            this.hideAllViews();

            this.initializeEvents();
            this.initializeContextMenu();
            this.initializeTaskbarIcon();
            this.initializeWindows();
            this.initializeCustomNotifications();
            this.initializeSyncingIndicator();

            this.startHook.KeyPressed += this.onGlobalStartKeyPressed;
            this.showHook.KeyPressed += this.onGlobalShowKeyPressed;
            this.idleDetectionTimer.Tick += this.onIdleDetectionTimerTick;

            this.finalInitialisation();
        }

        #region setup

        private void initializeSyncingIndicator()
        {
            this.syncingIndicator = new SyncingIndicator();
            this.Chrome.AddToHeaderButtons(this.syncingIndicator);
        }

        private void initializeCustomNotifications()
        {
            new AutotrackerNotification(this.taskbarIcon, this);
        }

        private void initializeContextMenu()
        {
            foreach (var item in this.ContextMenu.Items)
            {
                var asMenuItem = item as MenuItem;
                if (asMenuItem != null)
                {
                    asMenuItem.CommandTarget = this;
                }
            }
        }

        private void hideAllViews()
        {
            foreach (var view in this.views)
            {
                view.Visibility = Visibility.Collapsed;
            }
        }

        private void initializeWindows()
        {
            this.childWindows = new Window[]{
                this.editPopup = new EditViewPopup(),
                this.aboutWindow = new AboutWindow(),
                this.feedbackWindow = new FeedbackWindow(),
                new PreferencesWindow(),
            };
            this.idleNotificationWindow = new IdleNotificationWindow();

            this.editPopup.EditView.SetTimer(this.timerEntryListView.Timer);
            this.timerEntryListView.Timer.RunningTimeEntrySecondPulse += this.updateTaskbarTooltip;

            this.editPopup.IsVisibleChanged += this.editPopupVisibleChanged;
            this.editPopup.SizeChanged += (sender, args) => this.updateEntriesListWidth();

            this.idleNotificationWindow.AddedIdleTimeAsNewEntry += (o, e) => this.ShowOnTop();

            this.IsVisibleChanged += this.ownChildWindows;
        }

        private void initializeTaskbarIcon()
        {
            this.taskbarIcon.ContextMenu = this.mainContextMenu;
            this.taskbarIcon.Icon = new Icon(Properties.Resources.toggltray_inactive, SystemInformation.SmallIconSize);
            this.taskbarIcon.Visibility = Visibility.Visible;
        }

        private void ownChildWindows(object sender, DependencyPropertyChangedEventArgs args)
        {
            foreach (var window in this.childWindows)
            {
                window.Owner = this;
            }

            this.IsVisibleChanged -= this.ownChildWindows;
        }

        private void initializeEvents()
        {
            Toggl.OnApp += this.onApp;
            Toggl.OnError += this.onError;
            Toggl.OnLogin += this.onLogin;
            Toggl.OnTimeEntryList += this.onTimeEntryList;
            Toggl.OnOnlineState += this.onOnlineState;
            Toggl.OnReminder += this.onReminder;
            Toggl.OnURL += this.onURL;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
            Toggl.OnSettings += this.onSettings;
        }

        private void finalInitialisation()
        {
            Toggl.RegisterMainWindow(this);

            if (!Toggl.StartUI(Program.Version()))
            {
                MessageBox.Show(null, "Missing callback. See the log file for details");
                this.shutdown(1);
                return;
            }

            Utils.LoadWindowLocation(this, this.editPopup);
            

            this.aboutWindow.UpdateReleaseChannel();

            this.errorBar.Hide();
            this.statusBar.Hide();
            this.syncingIndicator.Hide();

            this.runScriptAsync();
        }

        private async void runScriptAsync()
        {
            await Task.Delay(TimeSpan.FromSeconds(1));

            if (Toggl.ScriptPath == null)
            {
                return;
            }

            ThreadPool.QueueUserWorkItem(args => this.runScript(), null);
        }

        private void runScript()
        {
            if (!File.Exists(Toggl.ScriptPath))
            {
                Toggl.Debug("Script file does not exist: " + Toggl.ScriptPath);
                this.shutdown(0);
            }

            var script = File.ReadAllText(Toggl.ScriptPath);

            long errorCode = 0;
            var result = Toggl.RunScript(script, ref errorCode);
            if (errorCode != 0)
            {
                Toggl.Debug(string.Format("Failed to run script, err = {0}", errorCode));
            }
            Toggl.Debug(result);

            if (errorCode == 0)
            {
                this.shutdown(0);
            }
        }

        #endregion

        #region toggl events

        private void onStoppedTimerState()
        {
            if (this.TryBeginInvoke(this.onStoppedTimerState))
                return;

            this.updateTracking(null);
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            if (this.TryBeginInvoke(this.onRunningTimerState, te))
                return;

            this.updateTracking(te);
        }

        private void onURL(string url)
        {
            Process.Start(url);
        }

        private void onReminder(string title, string informativeText)
        {
            if (this.TryBeginInvoke(this.onReminder, title, informativeText))
                return;

            this.taskbarIcon.ShowBalloonTipWithLargeIcon(title, informativeText, Properties.Resources.toggl);
        }

        private void onOnlineState(Toggl.OnlineState state)
        {
            if (this.TryBeginInvoke(this.onOnlineState, state))
                return;

            this.updateStatusIcons(state == Toggl.OnlineState.Online);
        }

        private void onTimeEntryList(bool open, List<Toggl.TogglTimeEntryView> list)
        {
            if (this.TryBeginInvoke(this.onTimeEntryList, open, list))
                return;

            if (open)
            {
                this.errorBar.Hide();
                this.setActiveView(this.timerEntryListView);
            }
        }

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open)
            {
                this.setActiveView(this.loginView);
            }

            if (open || userID == 0)
            {
                this.emailAddressMenuText.Text = "Logged out";
            }
            else
            {
                this.emailAddressMenuText.Text = Toggl.UserEmail();
            }
            this.updateTracking(null);
        }

        private void onError(string errmsg, bool userError)
        {
            if (this.TryBeginInvoke(this.onError, errmsg, userError))
                return;

            this.errorBar.ShowError(errmsg);
        }

        private void onApp(bool open)
        {
            if (this.TryBeginInvoke(this.onApp, open))
                return;

            if (open)
            {
                this.Show();
            }
        }

        private void onSettings(bool open, Toggl.TogglSettingsView settings)
        {
            if (this.TryBeginInvoke(this.onSettings, open, settings))
                return;

            this.setGlobalShortcutsFromSettings();
            this.idleDetectionTimer.IsEnabled = settings.UseIdleDetection;
            this.Topmost = settings.OnTop;
            this.setManualMode(settings.ManualMode, true);
        }

        #endregion

        #region ui events

        protected override void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.minimizeToTray();
        }

        protected override void onCogButtonClick(object sender, RoutedEventArgs e)
        {
            this.mainContextMenu.PlacementTarget = (FrameworkElement)sender;
            this.mainContextMenu.Placement = PlacementMode.Bottom;
            this.mainContextMenu.HorizontalOffset = 0;
            this.mainContextMenu.VerticalOffset = 0;

            this.Chrome.CogButton.IsEnabled = false;
            this.mainContextMenu.IsOpen = true;
        }

        protected override void onIconButtonClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser();
        }

        protected override void OnLocationChanged(EventArgs e)
        {
            this.updateEditPopupLocation();

            base.OnLocationChanged(e);
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            this.updateEditPopupLocation();

            base.OnRenderSizeChanged(sizeInfo);
        }

        protected override void OnStateChanged(EventArgs e)
        {
            this.resizeHandle.ShowOnlyIf(this.WindowState != WindowState.Maximized);

            base.OnStateChanged(e);
        }

        protected override void OnActivated(EventArgs e)
        {
            Toggl.SetWake();

            base.OnActivated(e);
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            e.Cancel = true;
            this.shutdown(0);
        }

        private void onMainContextMenuClosed(object sender, RoutedEventArgs e)
        {
            this.Chrome.CogButton.IsEnabled = true;
        }

        private void onGlobalShowKeyPressed(object sender, KeyPressedEventArgs e)
        {
            this.togglVisibility();
        }

        private void onGlobalStartKeyPressed(object sender, KeyPressedEventArgs e)
        {
            if (this.isTracking)
            {
                using (Performance.Measure("stopping time entry from global short cut", this.isInManualMode))
                {
                    Toggl.Stop();
                }
            }
            else
            {
                using (Performance.Measure("starting time entry from global short cut, manual mode: {0}", this.isInManualMode))
                {
                    this.startTimeEntry(true);
                }
            }
        }

        private void editPopupVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            this.updateEditPopupLocation();
            this.updateEntriesListWidth();
        }

        private void onTaskbarLeftMouseUp(object sender, RoutedEventArgs e)
        {
            this.togglVisibility();
        }

        private void onTrayBalloonTipClicked(object sender, RoutedEventArgs e)
        {
            this.ShowOnTop();
        }

        private void onResizeHandleLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (this.isResizingWithHandle)
                return;

            const int htBottomRight = 17;

            Mouse.Capture(null);

            Win32.SendMessage(this.interopHelper.Handle,
                Win32.wmNcLButtonDown,
                htBottomRight,
                0);

            this.resizeHandle.CaptureMouse();
            this.isResizingWithHandle = true;
        }

        private void onResizeHandleLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.endHandleResizing();
        }

        private void onWindowMouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Released)
            {
                this.endHandleResizing();
            }
        }

        private void onIdleDetectionTimerTick(object sender, EventArgs e)
        {
            Win32.LASTINPUTINFO lastInputInfo;
            lastInputInfo.cbSize = Win32.LASTINPUTINFO.SizeOf;
            lastInputInfo.dwTime = 0;
            if (!Win32.GetLastInputInfo(out lastInputInfo))
            {
                return;
            }
            var idleSeconds = (Environment.TickCount - lastInputInfo.dwTime) / 1000;
            if (idleSeconds < 1)
            {
                return;
            }
            Toggl.SetIdleSeconds((ulong)idleSeconds);
        }

        private void updateTaskbarTooltip(object sender, string s)
        {
            this.taskbarIcon.ToolTipText = this.trackingTitle + s;
        }

        #endregion

        #region command events

        private void onNewCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("starting time entry from menu, manual mode: {0}", this.isInManualMode))
            {
                this.startTimeEntry();
            }
        }

        private void onContinueCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("continuing time entry from menu"))
            {
                Toggl.ContinueLatest();
            }
        }
        
        private void onStopCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("stopping time entry from menu"))
            {
                Toggl.Stop();   
            }
        }
        
        private void onShowCommand(object sender, RoutedEventArgs e)
        {
            this.ShowOnTop();
        }
        
        private void onSyncCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("syncing from menu"))
            {
                Toggl.Sync();
            }
        }
        
        private void onReportsCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("opening reports from menu"))
            {
                Toggl.OpenInBrowser();
            }
        }
        
        private void onPreferencesCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("opening preferences from menu"))
            {
                Toggl.EditPreferences();
            }
        }
        
        private void onToggleManualModeCommand(object sender, RoutedEventArgs e)
        {
            this.setManualMode(!this.isInManualMode);
        }

        private void onClearCacheCommand(object sender, RoutedEventArgs e)
        {
            var result = MessageBox.Show(this,
                "This will remove your Toggl user data from this PC and log you out of the Toggl Desktop app. " +
                "Any unsynced data will be lost.\n\nDo you want to continue?", "Clear Cache",
                MessageBoxButton.OKCancel, "CLEAR CACHE");

            if (result == MessageBoxResult.OK)
            {
                using (Performance.Measure("clearing cache from menu"))
                {
                    Toggl.ClearCache();
                }
            }
        }
        
        private void onSendFeedbackCommand(object sender, RoutedEventArgs e)
        {
            this.feedbackWindow.Show();
            this.feedbackWindow.Activate();
        }
        
        private void onAboutCommand(object sender, RoutedEventArgs e)
        {
            this.aboutWindow.Show();
            this.feedbackWindow.Activate();
        }
        
        private void onLogoutCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("logging out from menu"))
            {
                Toggl.Logout();
            }
        }

        private void onQuitCommand(object sender, RoutedEventArgs e)
        {
            this.shutdown(0);
        }

        private void onEditRunningCommand(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("edit running entry with shortcut"))
            {
                if (this.isInManualMode)
                {
                    this.startTimeEntry();
                }
                else
                {
                    Toggl.Edit(null, true, null);
                }
            }
        }

        private void onHideCommand(object sender, ExecutedRoutedEventArgs e)
        {
            this.minimizeToTray();
        }

        #endregion

        #region canExecutes

        private void canExecuteShowCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = this.canBeShown();
        }
        private void canExecuteHideCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !this.canBeShown();
        }
        private void canExecuteNewCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn;
        }
        private void canExecuteContinueCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn && !this.isTracking;
        }
        private void canExecuteStopCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn && this.isTracking;
        }
        private void canExecuteSyncCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn;
        }
        private void canExecuteReportsCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn;
        }
        private void canExecuteToggleManualModeCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn && (this.isInManualMode || !this.isTracking);
        }
        private void canExecuteClearCacheCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn;
        }
        private void canExecuteLogoutCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn;
        }
        private void canExecuteEditRunningCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Program.IsLoggedIn && (this.isTracking || this.isInManualMode);
        }

        private bool canBeShown()
        {
            return !this.IsVisible
                || this.WindowState == WindowState.Minimized;
        }

        #endregion
        
        #region ui controlling

        private void setManualMode(bool manualMode, bool fromApi = false)
        {
            this.isInManualMode = manualMode;

            this.togglManualModeMenuItem.Header =
                this.isInManualMode ? "Use timer" : "Use manual mode";

            this.timerEntryListView.SetManualMode(this.isInManualMode);

            if (!fromApi)
            {
                Toggl.SetManualMode(manualMode);
            }
        }

        private void togglVisibility()
        {
            if (this.canBeShown())
            {
                this.ShowOnTop();
            }
            else
            {
                this.minimizeToTray();
            }
        }

        private void minimizeToTray()
        {
            this.Hide();
            this.closeEditPopup();
        }

        private void setGlobalShortcutsFromSettings()
        {
            try
            {
                this.startHook.ChangeTo(
                    Toggl.GetKeyModifierStart(), Toggl.GetKeyStart()
                    );
            }
            catch (Exception e)
            {
                Toggl.Debug("Could not register start shortcut: " + e);
            }

            try
            {
                this.showHook.ChangeTo(
                    Toggl.GetKeyModifierShow(), Toggl.GetKeyShow()
                    );
            }
            catch (Exception e)
            {
                Toggl.Debug("Could not register show hotkey: " + e);
            }
        }

        private void startTimeEntry(bool continueIfNotInManualMode = false)
        {
            if (this.isInManualMode)
            {
                var guid = Toggl.Start("", "0", 0, 0, "", "");
                Toggl.Edit(guid, false, Toggl.Duration);
            }
            else
            {
                if (continueIfNotInManualMode)
                {
                    Toggl.ContinueLatest();
                }
                else
                {
                    Toggl.Start("", "", 0, 0, "", "");   
                }
            }
        }

        public void ShowOnTop()
        {
            this.Show();
            if (this.WindowState == WindowState.Minimized)
                this.WindowState = WindowState.Normal;
            this.Activate();
        }

        private void shutdown(int exitCode)
        {
            if (this.closing)
                return;

            this.closing = true;

            this.PrepareShutdown(exitCode == 0);

            this.Close();

            this.Dispatcher.BeginInvoke(DispatcherPriority.Background, new Action(() =>
            {
                Program.Shutdown(exitCode);
            }));
        }

        public void PrepareShutdown(bool saveWindowLocation)
        {
            if (this.taskbarIcon != null)
            {
                this.taskbarIcon.Visibility = Visibility.Collapsed;
            }

            if (this.mainContextMenu != null)
            {
                this.mainContextMenu.IsOpen = false;
                this.mainContextMenu.Visibility = Visibility.Collapsed;
            }

            if (this.IsVisible)
            {
                if (saveWindowLocation)
                {
                    Utils.SaveWindowLocation(this, this.editPopup);
                }
                this.Hide();
            }
        }

        private void updateStatusIcons(bool isOnline)
        {
            Icon icon;

            if (this.isTracking)
            {
                icon = isOnline
                    ? Properties.Resources.toggltray
                    : Properties.Resources.toggltray_offline_active;
            }
            else
            {
                icon = isOnline
                    ? Properties.Resources.toggltray_inactive
                    : Properties.Resources.toggltray_offline_inactive;
            }

            this.taskbarIcon.Icon = new Icon(icon, SystemInformation.SmallIconSize);
        }

        private void updateTracking(Toggl.TogglTimeEntryView? timeEntry)
        {
            var tracking = timeEntry != null;

            this.isTracking = tracking;

            if (tracking)
            {
                var description = timeEntry.Value.Description;

                if (string.IsNullOrEmpty(description))
                {
                    this.Title = "Toggl Desktop";
                    this.runningMenuText.Text = "Timer is tracking";
                    this.trackingTitle = "";
                }
                else
                {
                    this.Title = description + " - Toggl Desktop";
                    this.runningMenuText.Text = description;
                    this.trackingTitle = description + " - ";
                }


                if(this.isInManualMode)
                    this.setManualMode(false);
            }
            else
            {
                this.runningMenuText.Text = "Timer is not tracking";
                this.Title = "Toggl Desktop";
                this.taskbarIcon.ToolTipText = "Toggl Desktop";
            }

            this.updateStatusIcons(true);
            this.SetIconState(tracking);
        }

        private void closeEditPopup()
        {
            if (this.editPopup.IsVisible)
            {
                // TODO: consider saving popup open state and restoring when window is shown
                this.editPopup.ClosePopup();
                this.timerEntryListView.DisableHighlight();
            }
        }

        #endregion

        #region window size, position and state handling

        private void endHandleResizing()
        {
            if (!this.isResizingWithHandle)
                return;

            Mouse.Capture(null);
            this.isResizingWithHandle = false;
        }

        private void updateEntriesListWidth()
        {
            if (this.WindowState == WindowState.Maximized && this.editPopup.IsVisible)
            {
                this.timerEntryListView.SetListWidth(this.ActualWidth - this.editPopup.ActualWidth);
            }
            else
            {
                this.timerEntryListView.DisableListWidth();
            }
        }

        private void updateEditPopupLocation()
        {
            if (this.editPopup == null || !this.editPopup.IsVisible)
                return;

            if (this.WindowState == WindowState.Maximized)
            {
                Win32.Rectangle bounds;
                Win32.GetWindowRect(this.interopHelper.Handle, out bounds);

                var x = (double)bounds.Left;
                var y = (double)bounds.Top;

                var headerHeight = this.WindowHeaderHeight + this.timerEntryListView.TimerHeight;

                y += headerHeight;
                x += this.ActualWidth;

                this.editPopup.SetPlacement(true, x, y, this.ActualHeight - headerHeight,
                    this.ActualWidth - 300, true);
            }
            else
            {
                var s = this.getCurrentScreen();
                bool left = s.WorkingArea.Right - (this.Left + this.ActualWidth) < this.editPopup.Width;

                var x = this.Left;
                var y = this.Top;

                if (!left)
                {
                    x += this.ActualWidth;
                }

                this.editPopup.SetPlacement(left, x, y, this.Height, s.WorkingArea.Width * 0.5);
            }

        }

        private void setActiveView(UserControl activeView)
        {
            if (activeView == null)
                throw new ArgumentNullException("activeView");

            if (this.activeView != null)
            {
                this.activeView.Visibility = Visibility.Collapsed;
            }

            this.activeView = activeView;

            activeView.Visibility = Visibility.Visible;
            this.closeEditPopup();

            this.updateMinimumSize(activeView);
        }

        private void updateMinimumSize(UserControl activeView)
        {
            this.MinHeight = this.WindowHeaderHeight + activeView.MinHeight;
            this.MinWidth = activeView.MinWidth;
        }

        #endregion

        private void changeThemeTest(object sender, RoutedEventArgs e)
        {
            Theme.Activate(ThemeTypes.Layout, this.themeCheckBox.IsChecked ?? false ? "Compact" : "Comfortable");
        }
    }
}
