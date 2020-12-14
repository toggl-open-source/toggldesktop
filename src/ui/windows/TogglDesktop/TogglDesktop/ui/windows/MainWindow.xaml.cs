using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using DynamicData.Binding;
using Microsoft.Win32;
using NHotkey;
using NHotkey.Wpf;
using ReactiveUI;
using TogglDesktop.Diagnostics;
using TogglDesktop.Services;
using TogglDesktop.Theming;
using TogglDesktop.Tutorial;
using TogglDesktop.ViewModels;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;
using MenuItem = System.Windows.Controls.MenuItem;

namespace TogglDesktop
{
    public partial class MainWindow
    {
        #region fields

        private const int WindowHeaderHeight = 32;
        private readonly DispatcherTimer idleDetectionTimer =
            new DispatcherTimer { Interval = TimeSpan.FromSeconds(1) };

        private readonly WindowInteropHelper interopHelper;
        private readonly IMainView[] views;
        private Window[] childWindows;

        private EditViewPopup editPopup;
        private IdleNotificationWindow idleNotificationWindow;
        private SyncingIndicator syncingIndicator;
        private MiniTimerWindow miniTimer;
        private InAppNotification inAppNotification;
        private NotificationManager notificationManager;

        private IMainView activeView;
        private bool closing;

        #endregion

        public MainWindow()
        {
            this.DataContext = this;
            this.InitializeComponent();

            KeyboardShortcuts.SetMainWindow(this);

            this.interopHelper = new WindowInteropHelper(this);

            this.views = new IMainView[] { this.overlayView, this.loginView, SSOLoginView, this.timerEntryListView };

            this.hideAllViews();

            this.initializeColorScheme();
            this.initializeEvents();
            this.initializeContextMenu();
            this.initializeTaskbarIcon();
            this.initializeWindows();
            this.initializeCustomNotifications();
            this.initializeSyncingIndicator();
            this.initializeTutorialManager();
            this.initializeSessionNotification();

            this.idleDetectionTimer.Tick += this.onIdleDetectionTimerTick;

            this.finalInitialisation();
            this.trackingWindowSize();
            InitializeViews();
            this.Loaded += onMainWindowLoaded;
        }

        #region properties

        public bool IsTracking { get; private set; }
        public bool IsInManualMode { get; private set; }
        public bool IsMiniTimerVisible { get; private set; }

        public TutorialManager TutorialManager { get; private set; }

        public bool CanBeShown => !this.IsVisible || !this.IsActive;

        public bool CanBeHidden => this.IsVisible && this.WindowState != WindowState.Minimized;

        public CogwheelMenuViewModel CogwheelMenuViewModel { get; private set; }

        #endregion

        #region setup

        private void initializeTutorialManager()
        {
            this.TutorialManager = new TutorialManager(this, this.timerEntryListView.Timer, this.tutorialPanel);
        }
        private void initializeSyncingIndicator()
        {
            this.syncingIndicator = new SyncingIndicator();
        }

        private void initializeCustomNotifications()
        {
            this.notificationManager = new NotificationManager(this.taskbarIcon, this);
        }

        private void initializeContextMenu()
        {
            CogwheelMenuViewModel = new CogwheelMenuViewModel();
            mainContextMenu.DataContext = CogwheelMenuViewModel;
            mainContextMenu.Initialize(this);

            ContextMenu = mainContextMenu;
        }

        private void initializeSessionNotification()
        {
            SystemEvents.SessionSwitch += SystemEvents_SessionSwitch;
            SystemEvents.SessionEnding += SystemEventsOnSessionEnding;
            SystemEvents.PowerModeChanged += SystemEventsOnPowerModeChanged;
        }

        private void SystemEventsOnSessionEnding(object sender, SessionEndingEventArgs e)
        {
            if (e.Reason == SessionEndReasons.SystemShutdown ||
                e.Reason == SessionEndReasons.Logoff)
            {
                Toggl.SetOSShutdown();
            }
        }

        private void SystemEventsOnPowerModeChanged(object sender, PowerModeChangedEventArgs e)
        {
            if (e.Mode == PowerModes.Suspend)
            {
                Toggl.SetSleep();
            }
            else if (e.Mode == PowerModes.Resume)
            {
                Toggl.SetWake();
            }
        }

        private void SystemEvents_SessionSwitch(object sender, SessionSwitchEventArgs e)
        {
            if (e.Reason == SessionSwitchReason.SessionLock)
            {
                Toggl.SetLocked();
            }
            else if (e.Reason == SessionSwitchReason.SessionUnlock)
            {
                Toggl.SetUnlocked();
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
            var aboutWindowViewModel = new AboutWindowViewModel(
                updateService: Toggl.UpdateService,
                versionText: $"Version {Program.Version()} {Utils.Bitness()}");

            this.childWindows = new Window[]{
                this.editPopup = new EditViewPopup(),
                new AboutWindow(aboutWindowViewModel),
                new FeedbackWindow(),
                new PreferencesWindow(),
            };
            this.idleNotificationWindow = new IdleNotificationWindow();

            this.editPopup.EditView.SetTimer(this.timerEntryListView.Timer);
            this.timerEntryListView.Timer.ViewModel.WhenValueChanged(x => x.DurationText).Subscribe(x => updateTaskbarTooltip(this, x));
            this.timerEntryListView.Timer.StartStopButtonClicked += ()  => closeEditPopup(true);
            this.timerEntryListView.Entries.SetEditPopup(this.editPopup);
            this.timerEntryListView.Entries.CloseEditPopup += (sender, args) => this.closeEditPopup(true);

            this.editPopup.IsVisibleChanged += this.editPopupVisibleChanged;
            editPopup.Activated += (s, e) => updateEditPopupLocation();
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

        private void initializeColorScheme()
        {
            Theme.CurrentColorScheme.Subscribe(x => this.updateTitleBarBackground(activeView));
            Theme.CurrentColorScheme.Subscribe(x =>
            {
                this.taskbarIcon.TrayToolTip = null;
                this.taskbarIcon.TrayToolTip = trayToolTip;
            });
        }

        private void initializeEvents()
        {
            Toggl.OnApp += this.onApp;
            Toggl.OnOverlay += this.onOverlay;
            Toggl.OnError += this.onError;
            Toggl.OnLogin += this.onLogin;
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnTimeEntryList += this.onTimeEntryList;
            Toggl.OnOnlineState += this.onOnlineState;
            Toggl.OnURL += this.onURL;
            Toggl.OnUserTimeEntryStart += this.onUserTimeEntryStart;
            Toggl.RunningTimeEntry.Subscribe(this.onRunningTimeEntry);
            Toggl.OnSettings += this.onSettings;
            Toggl.OnDisplayInAppNotification += this.onDisplayInAppNotification;
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

            this.loadPositions();

            var updateChannelFromLocalDb = Toggl.UpdateChannel();
            Toggl.UpdateService.UpdateChannel.OnNext(updateChannelFromLocalDb);

            this.errorBar.Hide();
            this.statusBar.Hide();
            this.syncingIndicator.Hide();

            this.SetMiniTimerVisible(Toggl.GetMiniTimerVisible(), true);
        }

        public void loadPositions()
        {
            Utils.LoadWindowLocation(this, this.editPopup, this.miniTimer);
        }

        private void trackingWindowSize()
        {
            var currentWindow = this.childWindows.First();
            if (currentWindow == null)
            {
                return;
            }
            Toggl.TrackWindowSize(new System.Windows.Size(currentWindow.Width, currentWindow.Height));
        }

        #endregion

        #region toggl events

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, te, focusedFieldName))
                return;

            this.updateEditPopupLocation(true);

            if (open)
            {
                this.Show();
            }
        }

        private void onRunningTimeEntry(Toggl.TogglTimeEntryView? te)
        {
            if (this.TryBeginInvoke(this.onRunningTimeEntry, te))
                return;

            this.updateTracking(te);
        }

        private void onUserTimeEntryStart()
        {
            this.taskbarIcon.CloseBalloon();
        }

        private void onURL(string url)
        {
            try
            {
                Process.Start(url);
            }
            catch (Exception e)
            {
                if (!Utils.TryOpenInDefaultBrowser(url))
                {
                    Toggl.ShowErrorAndNotify("Wasn't able to open the browser", e);
                }
            }
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

            // Get Today's total duration
            var i = 0;
            for (; i < list.Count; i++)
            {
                if (list[i].DateHeader == "Today") {
                    this.trayToolTip.TotalToday = list[i].DateDuration;
                    return;
                }
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
                CogwheelMenuViewModel.Email = "Logged out";
                this.taskbarIcon.TrayToolTip = null;
                this.taskbarIcon.ToolTipText = "Toggl - Logged out";
                this.SetMiniTimerVisible(false);
            }
            else
            {
                CogwheelMenuViewModel.Email = Toggl.UserEmail();
                this.taskbarIcon.TrayToolTip = trayToolTip;
                this.taskbarIcon.ToolTipText = $"Toggl - Logged in as {Toggl.UserEmail()}";
            }

            this.updateTracking(null);
        }

        private void OpenSSOLoginView()
        {
            setActiveView(SSOLoginView);
        }

        private void OpenLoginView(string confirmationCode, string email)
        {
            loginView.ViewModel.SetLinkSSOMode(confirmationCode, email);
            setActiveView(loginView);
        }

        private void onOverlay(long type)
        {

            if (this.TryBeginInvoke(this.onOverlay, type))
                return;

            this.overlayView.setType((int)type);
            this.setActiveView(this.overlayView);
        }

        private void onError(string errmsg, bool userError)
        {
            if (this.TryBeginInvoke(this.onError, errmsg, userError))
                return;

            if (this.activeView?.TryShowErrorInsideView(errmsg) != true)
            {
                this.errorBar.ShowError(errmsg);
            }
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

            Theme.SetThemeFromSettings(settings.ColorTheme);
            this.setGlobalShortcutsFromSettings();
            this.idleDetectionTimer.IsEnabled = settings.UseIdleDetection;
            this.Topmost = settings.OnTop;
            this.SetManualMode(settings.ManualMode, true);
        }

        private void onDisplayInAppNotification(string title, string text, string button, string url)
        {
            if (this.TryBeginInvoke(this.onDisplayInAppNotification, title, text, button, url))
                return;

            if (inAppNotification == null)
            {
                inAppNotification = new InAppNotification
                {
                    Visibility = Visibility.Collapsed
                };
                root.Children.Add(inAppNotification);
            }

            inAppNotification.Title = title;
            inAppNotification.Text = text;
            inAppNotification.Button = button;
            inAppNotification.Url = url;

            inAppNotification.RunAppearAnimation();
        }

        #endregion

        #region ui events

        protected void onCogButtonClick(object sender, RoutedEventArgs e)
        {
            var button = (FrameworkElement) sender;
            this.mainContextMenu.PlacementTarget = button;
            this.mainContextMenu.Placement = PlacementMode.Bottom;
            this.mainContextMenu.HorizontalOffset = 0;
            this.mainContextMenu.VerticalOffset = 0;
            // disable button so that second button click in a row does not bring up context menu as soon as it's closed
            this.cogButton.IsEnabled = false;
            this.mainContextMenu.IsOpen = true;
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

        protected override void OnClosing(CancelEventArgs e)
        {
            e.Cancel = true;
            this.MinimizeToTray();
        }

        private void onMainContextMenuClosed(object sender, RoutedEventArgs e)
        {
            this.cogButton.IsEnabled = true;
        }

        private void onGlobalShowKeyPressed(object sender, HotkeyEventArgs args)
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

        private void onGlobalStartKeyPressed(object sender, HotkeyEventArgs args)
        {
            if (this.IsTracking)
            {
                using (Performance.Measure("stopping time entry from global short cut", this.IsInManualMode))
                {
                    Toggl.Stop();
                }
            }
            else
            {
                using (Performance.Measure("starting time entry from global short cut, manual mode: {0}", this.IsInManualMode))
                {
                    KeyboardShortcuts.StartTimeEntry(true);
                }
            }
        }

        private void editPopupVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            this.updateEntriesListWidth();
            if (editPopup.IsVisible == false && ReferenceEquals(this.activeView, this.timerEntryListView))
            {
                this.timerEntryListView.DeselectTimeEntries();
            }
        }

        private void onTaskbarLeftMouseUp(object sender, RoutedEventArgs e)
        {
            if (this.CanBeHidden)
            {
                this.MinimizeToTray();
            }
            else
            {
                this.ShowOnTop();
            }
        }

        private void onTrayBalloonTipClicked(object sender, RoutedEventArgs e)
        {
            this.ShowOnTop();
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
            if (this.TryBeginInvoke(updateTaskbarTooltip, sender, s))
                return;

            this.trayToolTip.SetDuration(s);
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
            this.IsMiniTimerVisible = visible;

            CogwheelMenuViewModel.IsMiniTimerUsed = visible;

            this.miniTimer.SetVisible(visible);

            if (visible)
            {
                // Make sure minitimer is not off screen
                Utils.CheckMinitimerVisibility(this.miniTimer);
            }

            if (!fromApi)
            {
                Toggl.SetMiniTimerVisible(visible);
            }
        }

        public void SetManualMode(bool manualMode, bool fromApi = false)
        {
            this.IsInManualMode = manualMode;

            CogwheelMenuViewModel.IsInManualMode = manualMode;

            this.timerEntryListView.SetManualMode(this.IsInManualMode);
            this.miniTimer.SetManualMode(this.IsInManualMode);

            if (!fromApi)
            {
                Toggl.SetManualMode(manualMode);
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
                var keyStart = Toggl.GetKeyStart();
                if (keyStart != Key.None)
                {
                    HotkeyManager.Current.AddOrReplace(
                        "Toggl.ContinueOrStop",
                        keyStart,
                        Toggl.GetKeyModifierStart(),
                        onGlobalStartKeyPressed);
                }
                else
                {
                    HotkeyManager.Current.Remove("Toggl.ContinueOrStop");
                }
            }
            catch (Exception e)
            {
                Toggl.Debug("Could not register start shortcut: " + e);
            }

            try
            {
                var keyShow = Toggl.GetKeyShow();
                if (keyShow != Key.None)
                {
                    HotkeyManager.Current.AddOrReplace(
                        "Toggl.ShowHideToggl",
                        keyShow,
                        Toggl.GetKeyModifierShow(),
                        onGlobalShowKeyPressed);
                }
                else
                {
                    HotkeyManager.Current.Remove("Toggl.ShowHideToggl");
                }
            }
            catch (Exception e)
            {
                Toggl.Debug("Could not register show hotkey: " + e);
            }
        }

        public void shutdown(int exitCode)
        {
            if (this.closing)
                return;

            this.closing = true;

            this.PrepareShutdown(exitCode == 0);

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
                this.taskbarIcon.Icon = null;
                this.taskbarIcon.Dispose();
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

            this.childWindows.ForEach(w => w.Hide());
            this.Close();
        }

        private void updateStatusIcons(bool isOnline)
        {
            Icon icon;

            if (this.IsTracking)
            {
                icon = Properties.Resources.toggltray;
            }
            else
            {
                icon = Properties.Resources.toggltray_inactive;
            }

            this.taskbarIcon.Icon = new Icon(icon, SystemInformation.SmallIconSize);
        }

        private void updateTracking(Toggl.TogglTimeEntryView? timeEntry)
        {
            var tracking = timeEntry != null;

            this.IsTracking = tracking;
            this.trayToolTip.IsTracking = tracking;

            if (tracking)
            {
                this.trayToolTip.TimeEntryLabel = timeEntry.Value.ToTrayToolTipTimeEntryLabelViewModel();
                this.trayToolTip.SetDuration(timeEntry.Value);

                var description = timeEntry.Value.Description;

                this.Title = string.IsNullOrEmpty(description)
                    ? "Toggl Track"
                    : description + " - Toggl Track";

                if (this.IsInManualMode)
                    this.SetManualMode(false);
            }
            else
            {
                this.trayToolTip.TimeEntryLabel = null;
                this.Title = "Toggl Track";
            }

            var iconName = this.IsTracking ? "toggl" : "toggl_inactive";
            this.Icon = BitmapFrame.Create(new Uri("pack://application:,,,/TogglDesktop;component/Resources/" + iconName + ".ico"));
            this.updateStatusIcons(true);
        }

        private void closeEditPopup(bool focusTimeEntryList = false, bool skipAnimation = false)
        {
            if (this.TryBeginInvoke(this.closeEditPopup, focusTimeEntryList, skipAnimation))
                return;

            if (this.editPopup != null && this.editPopup.IsVisible)
            {
                // TODO: consider saving popup open state and restoring when window is shown
                this.editPopup.ClosePopup(skipAnimation);
                if (focusTimeEntryList)
                {
                    Toggl.ViewTimeEntryList();
                }
                else if (this.activeView == this.timerEntryListView && !this.timerEntryListView.Entries.IsKeyboardFocusWithin)
                {
                    this.timerEntryListView.Timer.Focus();
                }
            }
        }

        #endregion

        #region window size, position and state handling

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

                var headerHeight = WindowHeaderHeight + this.timerEntryListView.TimerHeight + 8;

                y += headerHeight;
                x += this.ActualWidth;

                this.editPopup.SetPlacementMaximized(x, y, this.ActualHeight - headerHeight, this.ActualWidth - 300);
            }
            else
            {
                var s = this.GetCurrentScreenRectangle();
                bool left = s.Right - (this.Left + this.ActualWidth) < this.editPopup.Width;

                var x = this.Left;
                var y = Top + editPopup.Height > s.Bottom ? s.Bottom - editPopup.Height : Top;

                if (!left)
                {
                    x += this.ActualWidth;
                }

                this.editPopup.SetPlacement(left, x, y, s.Width * 0.5);
            }

        }

        private void setActiveView(IMainView activeView)
        {
            if (activeView == null)
                throw new ArgumentNullException(nameof(activeView));

            if (this.activeView != activeView)
            {
                var hadActiveView = this.activeView != null;

                if (hadActiveView)
                {
                    this.activeView.Deactivate(true);
                }

                this.activeView = activeView;
                this.activeView.Activate(hadActiveView);
                if (hadActiveView)
                {
                    Action focusWindowAction = this.ShowOnTop;
                    this.Dispatcher.BeginInvoke(DispatcherPriority.SystemIdle, focusWindowAction);
                }
            }

            this.closeEditPopup();

            this.updateMinimumSize(activeView);
            this.updateTitleBarBackground(activeView);
        }

        private void InitializeViews()
        {
            loginView.ViewModel = new LoginViewModel(OpenSSOLoginView);
            SSOLoginView.ViewModel = new SSOLoginViewModel(OpenLoginView);
        }

        public void ProcessStartupUri(string uri)
        {
            SSOLoginView.ViewModel.AuthUri.OnNext(new Uri(uri));
        }

        private void updateMinimumSize(IMainView activeView)
        {
            this.MinHeight = WindowHeaderHeight + activeView.MinHeight;
            this.MinWidth = activeView.MinWidth;
        }

        private void updateTitleBarBackground(IMainView activeView)
        {
            if (activeView == null)
            {
                return;
            }
            this.WindowTitleBrush = activeView.TitleBarBrush;
            this.NonActiveWindowTitleBrush = activeView.TitleBarBrush;
        }

        private void onMainWindowLoaded(object sender, EventArgs args)
        {
            this.Loaded -= onMainWindowLoaded;
            this.ShowOnTop();
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

        private void OnMainWindowKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                this.closeEditPopup();
            }
        }
    }
}
