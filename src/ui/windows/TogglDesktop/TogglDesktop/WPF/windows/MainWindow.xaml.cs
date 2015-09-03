using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Interop;
using MessageBox = System.Windows.MessageBox;
using UserControl = System.Windows.Controls.UserControl;

namespace TogglDesktop.WPF
{
    public partial class MainWindow
    {
        private readonly WindowInteropHelper interopHelper;
        private readonly UserControl[] views;
        private Window[] childWindows;

        private AboutWindow aboutWindow;
        private FeedbackWindow feedbackWindow;
        private PreferencesWindow preferencesWindow;
        private EditViewPopup editPopup;

        private UserControl activeView;

        public MainWindow()
        {
            this.InitializeComponent();

            this.views = new UserControl[] {this.loginView, this.timerEntryListView};

            this.hideAllViews();

            this.initializeWindows();
            this.initializeEvents();

            this.finalInitialisation();

            this.interopHelper = new WindowInteropHelper(this);
        }

        #region setup

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
                this.aboutWindow = new AboutWindow(),
                this.feedbackWindow = new FeedbackWindow(),
                this.preferencesWindow = new PreferencesWindow(),
                this.editPopup = new EditViewPopup(),
            };

            this.timerEntryListView.SetEditPopup(this.editPopup.EditView);

            this.editPopup.IsVisibleChanged += this.editPopupVisibleChanged;
            this.editPopup.SizeChanged += (sender, args) => this.updateEntriesListWidth();

            this.IsVisibleChanged += this.ownChildWindows;
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
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnOnlineState += this.onOnlineState;
            Toggl.OnReminder += this.onReminder;
            Toggl.OnURL += this.onURL;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
            Toggl.OnSettings += this.onSettings;
            Toggl.OnIdleNotification += this.onIdleNotification;
        }

        private void finalInitialisation()
        {
            if (!Toggl.StartUI(TogglDesktop.Program.Version()))
            {
                MessageBox.Show("Missing callback. See the log file for details");
                Program.Shutdown(1);
            }

            Utils.LoadWindowLocation(this, this.editPopup);

            this.aboutWindow.UpdateReleaseChannel();

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
                TogglDesktop.Program.Shutdown(0);
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
                TogglDesktop.Program.Shutdown(0);
            }
        }

        #endregion


        #region toggl events

        private void onIdleNotification(string guid, string since, string duration, ulong started, string description)
        {
        }

        private void onSettings(bool open, Toggl.TogglSettingsView settings)
        {
        }

        private void onStoppedTimerState()
        {
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
        }

        private void onURL(string url)
        {
        }

        private void onReminder(string title, string informativeText)
        {
        }

        private void onOnlineState(long state)
        {
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
        }

        private void onTimeEntryList(bool open, List<Toggl.TogglTimeEntryView> list)
        {
            if (this.TryBeginInvoke(this.onTimeEntryList, open, list))
                return;

            if (open)
            {
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
        }

        private void onError(string errmsg, bool userError)
        {
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

        #endregion

        #region ui events


        private void editPopupVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            this.updateEditPopupLocation();
            this.updateEntriesListWidth();
        }

        protected override void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            //TODO: replace by hiding

            Program.Shutdown(0);
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            Utils.SaveWindowLocation(this, this.editPopup);

            base.OnClosing(e);
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

                this.editPopup.SetPlacement(true, x, y, this.ActualHeight - headerHeight, true);
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

                this.editPopup.SetPlacement(left, x, y, this.Height);
            }

        }

        private Screen getCurrentScreen()
        {
            var bounds = new Rectangle(
                (int)this.Left, (int)this.Top, (int)this.Width, (int)this.Height
                );

            return Screen.AllScreens
                .FirstOrDefault(s => s.Bounds.IntersectsWith(bounds))
                ?? Screen.PrimaryScreen;
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
            this.editPopup.Hide();
            this.timerEntryListView.DisableHighlight();

            this.updateMinimumSize(activeView);
        }

        private void updateMinimumSize(UserControl activeView)
        {
            this.MinHeight = this.WindowHeaderHeight + activeView.MinHeight;
            this.MinWidth = activeView.MinWidth;
        }

        #endregion
    }
}
