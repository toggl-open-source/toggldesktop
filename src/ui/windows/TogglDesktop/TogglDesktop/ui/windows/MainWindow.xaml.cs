using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Threading;
using TogglDesktop.Diagnostics;
using TogglDesktop.Experiments;
using TogglDesktop.Tutorial;
using MenuItem = System.Windows.Controls.MenuItem;
using MouseEventArgs = System.Windows.Input.MouseEventArgs;

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
        private readonly IMainView[] views;
        private Window[] childWindows;

        private EditViewPopup editPopup;
        private IdleNotificationWindow idleNotificationWindow;
        private SyncingIndicator syncingIndicator;
        private ExperimentManager experimentManager;
        private MiniTimerWindow miniTimer;

        private IMainView activeView;
        private bool isInManualMode;
        private bool isMiniTimerVisible;
        private bool isTracking;
        private bool isResizingWithHandle;
        private bool closing;
        private string trackingTitle;
        private WindowState previousWindowState;

        #endregion

        public MainWindow()
        {
            this.DataContext = this;
            this.InitializeComponent();

            KeyboardShortcuts.SetMainWindow(this);

            this.interopHelper = new WindowInteropHelper(this);

            this.views = new IMainView[] { this.loginView, this.timerEntryListView };

            this.hideAllViews();

            this.initializeEvents();
            this.initializeContextMenu();
            this.initializeTaskbarIcon();
            this.initializeWindows();
            this.initializeCustomNotifications();
            this.initializeSyncingIndicator();
            this.initializeTutorialManager();
            this.initializeExperimentManager();

            this.startHook.KeyPressed += this.onGlobalStartKeyPressed;
            this.showHook.KeyPressed += this.onGlobalShowKeyPressed;
            this.idleDetectionTimer.Tick += this.onIdleDetectionTimerTick;

            this.finalInitialisation();
        }

        #region properties

        public bool IsTracking { get { return this.isTracking; } }
        public bool IsInManualMode { get { return this.isInManualMode; } }
        public bool IsMiniTimerVisible { get { return this.isMiniTimerVisible; } }

        public TutorialManager TutorialManager { get; private set; }

        public bool CanBeShown
        {
            get { return !this.IsVisible || this.WindowState == WindowState.Minimized; }
        }

        #endregion

        #region setup

        private void initializeExperimentManager()
        {
            this.experimentManager = new ExperimentManager(this);
        }

        private void initializeTutorialManager()
        {
            this.TutorialManager = new TutorialManager(this, this.timerEntryListView.Timer, this.tutorialPanel);
        }
        private void initializeSyncingIndicator()
        {
            this.syncingIndicator = new SyncingIndicator();
            this.Chrome.AddToHeaderButtons(this.syncingIndicator);
        }

        private void initializeCustomNotifications()
        {
            new AutotrackerNotification(this.taskbarIcon, this);
            new PomodoroNotification(this.taskbarIcon, this);
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
                view.Deactivate(false);
            }
        }

        private void initializeWindows()
        {
            this.childWindows = new Window[]{
                this.editPopup = new EditViewPopup(),
                new AboutWindow(),
                new FeedbackWindow(),
                new PreferencesWindow(),
            };
            this.idleNotificationWindow = new IdleNotificationWindow();

            this.editPopup.EditView.SetTimer(this.timerEntryListView.Timer);
            this.timerEntryListView.Timer.RunningTimeEntrySecondPulse += this.updateTaskbarTooltip;
            this.timerEntryListView.Timer.StartStopClick += (sender, args) => this.closeEditPopup(true);
            this.timerEntryListView.Entries.SetEditPopup(this.editPopup);
            this.timerEntryListView.Entries.CloseEditPopup += (sender, args) => this.closeEditPopup(true);

            this.editPopup.IsVisibleChanged += this.editPopupVisibleChanged;
            this.editPopup.SizeChanged += (sender, args) => this.updateEntriesListWidth();

            this.idleNotificationWindow.AddedIdleTimeAsNewEntry += (o, e) => this.ShowOnTop();

            this.miniTimer = new MiniTimerWindow(this);

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
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
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

            if (!Toggl.StartUI(Program.Version(), this.experimentManager.CurrentExperumentIds))
            {
                MessageBox.Show(null, "Missing callback. See the log file for details");
                this.shutdown(1);
                return;
            }

            Utils.LoadWindowLocation(this, this.editPopup, this.miniTimer);


            this.GetWindow<AboutWindow>().UpdateReleaseChannel();

            this.errorBar.Hide();
            this.statusBar.Hide();
            this.syncingIndicator.Hide();

            this.runScriptAsync();

            this.SetMiniTimerVisible(Toggl.GetMiniTimerVisible(), true);
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

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, te, focusedFieldName))
                return;

            this.updateEditPopupLocation(true);
        }

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

        private void onTimeEntryList(bool open, List<Toggl.TogglTimeEntryView> list, bool showLoadMoreButton)
        {
            if (this.TryBeginInvoke(this.onTimeEntryList, open, list, showLoadMoreButton))
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
            this.SetManualMode(settings.ManualMode, true);
        }

        #endregion

        #region ui events

        protected override void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.MinimizeToTray();
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

            // fix for when maximised window is activated from a different process using ShowWindow()
            if (this.fixWindowStateOnShowWindow())
                return;

            this.previousWindowState = this.WindowState;

            base.OnStateChanged(e);
        }

        protected override void OnActivated(EventArgs e)
        {
            // fix for when the window is activated from a different process using ShowWindow()
            this.fixVisibilityOnShowWindow();

            Toggl.SetWake();

            base.OnActivated(e);
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            e.Cancel = true;
            this.MinimizeToTray();
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
                    KeyboardShortcuts.StartTimeEntry(true);
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

        #region commands

        #region Escape

        private void onEscapeCommand(object sender, ExecutedRoutedEventArgs e)
        {
            this.closeEditPopup();
        }
        #endregion

        #endregion

        #region ui controlling

        public void SetMiniTimerVisible(bool visible, bool fromApi = false)
        {
            this.isMiniTimerVisible = visible;

            this.togglMiniTimerVisibilityMenuItem.IsChecked = visible;

            this.miniTimer.SetVisible(visible);

            if (!fromApi)
            {
                Toggl.SetMiniTimerVisible(visible);
            }
        }

        public void SetManualMode(bool manualMode, bool fromApi = false)
        {
            this.isInManualMode = manualMode;

            this.togglManualModeMenuItem.IsChecked = manualMode;

            this.timerEntryListView.SetManualMode(this.isInManualMode);
            this.miniTimer.SetManualMode(this.isInManualMode);

            if (!fromApi)
            {
                Toggl.SetManualMode(manualMode);
            }
        }

        private void togglVisibility()
        {
            if (this.CanBeShown)
            {
                this.ShowOnTop();
            }
            else
            {
                this.MinimizeToTray();
            }
        }

        public void MinimizeToTray()
        {
            Utils.SaveWindowLocation(this, this.editPopup, this.miniTimer);
            this.Hide();
            this.closeEditPopup(skipAnimation: true);
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

        public void ShowOnTop()
        {
            this.Show();
            if (this.WindowState == WindowState.Minimized)
            {
                this.WindowState = WindowState.Normal;
            }
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

            if (this.editPopup != null)
            {
                this.closeEditPopup(skipAnimation: true);
            }

            if (this.miniTimer != null && this.miniTimer.IsVisible)
            {
                this.miniTimer.Hide();
            }

            if (saveWindowLocation)
            {
                Utils.SaveWindowLocation(this, this.editPopup, this.miniTimer);
            }

            if (this.IsVisible)
            {
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


                if (this.isInManualMode)
                    this.SetManualMode(false);
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

        private void closeEditPopup(bool focusTimeEntryList = false, bool skipAnimation = false)
        {
            if (this.editPopup != null && this.editPopup.IsVisible)
            {
                // TODO: consider saving popup open state and restoring when window is shown
                this.editPopup.ClosePopup(skipAnimation);
                this.timerEntryListView.DisableHighlight();
                if (focusTimeEntryList)
                {
                    Toggl.ViewTimeEntryList();
                }
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

        private void updateEditPopupLocation(bool forceUpdate = false)
        {
            if (this.editPopup == null || (!forceUpdate && !this.editPopup.IsVisible))
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
                var s = this.getCurrentScreenRectangle();
                bool left = s.Right - (this.Left + this.ActualWidth) < this.editPopup.Width;

                var x = this.Left;
                var y = this.Top;

                if (!left)
                {
                    x += this.ActualWidth;
                }

                this.editPopup.SetPlacement(left, x, y, this.Height, s.Width * 0.5);
            }

        }

        private void setActiveView(IMainView activeView)
        {
            if (activeView == null)
                throw new ArgumentNullException("activeView");

            var hadActiveView = this.activeView != null;
            if (hadActiveView)
            {
                this.activeView.Deactivate(true);
            }

            this.activeView = activeView;

            this.activeView.Activate(hadActiveView);

            this.closeEditPopup();

            this.updateMinimumSize(activeView);
        }

        private void updateMinimumSize(IMainView activeView)
        {
            this.MinHeight = this.WindowHeaderHeight + activeView.MinHeight;
            this.MinWidth = activeView.MinWidth;
        }

        private bool fixWindowStateOnShowWindow()
        {
            if (this.Visibility != Visibility.Visible)
            {
                if (this.previousWindowState == WindowState.Maximized && this.WindowState != WindowState.Maximized)
                {
                    this.WindowState = WindowState.Maximized;
                    this.Visibility = Visibility.Visible;
                    return true;
                }
            }
            return false;
        }

        private void fixVisibilityOnShowWindow()
        {
            if (this.Visibility != Visibility.Visible)
            {
                this.Visibility = Visibility.Visible;
            }
        }


        #endregion

        public T GetWindow<T>()
            where T : Window
        {
            return (T)this.childWindows.First(w => w is T);
        }

        public T GetView<T>()
        {
            return (T)this.views.FirstOrDefault(v => v is T);
        }

        private void changeThemeTest(object sender, RoutedEventArgs e)
        {
            Theme.Activate(ThemeTypes.Layout, this.themeCheckBox.IsChecked ?? false ? "Compact" : "Comfortable");
        }

    }
}
