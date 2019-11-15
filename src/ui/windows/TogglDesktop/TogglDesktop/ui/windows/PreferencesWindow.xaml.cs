using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;
#if MS_STORE
using TogglDesktop.Win10;
#endif
namespace TogglDesktop
{
    partial class PreferencesWindow
    {
        private const string recordButtonIdleText = "Record shortcut";
        private const string recordButtonRecordingText = "Type shortcut...";

        private readonly ShortcutRecorder showHideShortcutRecorder;
        private readonly ShortcutRecorder continueStopShortcutRecorder;
        private bool isSaving;

        private Toggl.TogglAutocompleteView selectedDefaultProject;
        private List<Toggl.TogglAutocompleteView> knownProjects;

        public PreferencesWindow()
        {
            this.InitializeComponent();

            Toggl.OnSettings += this.onSettings;
            Toggl.OnLogin += this.onLogin;
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;

            this.showHideShortcutRecorder = new ShortcutRecorder(
                this.showHideShortcutRecordButton,
                this.showHideShortcutClearButton,
                this.shortcutErrorText,
                "Global Show/Hide"
                );
            this.continueStopShortcutRecorder = new ShortcutRecorder(
                this.continueStopShortcutRecordButton,
                this.continueStopShortcutClearButton,
                this.shortcutErrorText,
                "Global Continue/Stop"
                );

            var allShortcutRecorders = new List<ShortcutRecorder>
            {
                this.showHideShortcutRecorder, this.continueStopShortcutRecorder
            }.AsReadOnly();

            foreach (var recorder in allShortcutRecorders)
            {
                recorder.SetShortcutList(allShortcutRecorders);
            }

            this.IsVisibleChanged += (sender, args) => this.isVisibleChanged();
        }

        private void isVisibleChanged()
        {
            if (this.IsVisible)
            {
                return;
            }

            this.showHideShortcutRecorder.Reset();
            this.continueStopShortcutRecorder.Reset();
        }
        
        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            this.recordTimelineCheckBox.IsEnabled = !open && userID != 0;
            this.recordTimelineCheckBox.IsChecked = Toggl.IsTimelineRecordingEnabled();
        }

        private void onSettings(bool open, Toggl.TogglSettingsView settings)
        {
            if (this.TryBeginInvoke(this.onSettings, open, settings))
                return;

            if (this.isSaving)
                return;

            using (Performance.Measure("filling settings from OnSettings"))
            {
                this.updateUI(settings);
                this.selectDefaultProjectFromSettings();
            }

            if (open)
            {
                this.Show();
                this.Activate();
            }
        }
        private void onProjectAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onProjectAutocomplete, list))
                return;

            this.knownProjects = list;

            this.defaultProjectAutoComplete.SetController(AutoCompleteControllers.ForProjects(list));
        }


        private void updateUI(Toggl.TogglSettingsView settings)
        {
            #region general

            this.idleDetectionCheckBox.IsChecked = settings.UseIdleDetection;
            this.idleDetectionDurationTextBox.Text = settings.IdleMinutes.ToString();

            this.enablePomodoroCheckBox.IsChecked = settings.Pomodoro;
            this.pomodoroTimerDuration.Text = settings.PomodoroMinutes.ToString();

            this.enablePomodoroBreakCheckBox.IsChecked = settings.PomodoroBreak;
            this.pomodoroBreakTimerDuration.Text = settings.PomodoroBreakMinutes.ToString();

            this.recordTimelineCheckBox.IsChecked = settings.RecordTimeline;
            this.onTopCheckBox.IsChecked = settings.OnTop;

            this.keepEndTimeFixedCheckbox.IsChecked = Toggl.GetKeepEndTimeFixed();

            this.onStopEntryCheckBox.IsChecked = settings.StopEntryOnShutdownSleep;
            Task.Run(UpdateLaunchOnStartupCheckboxAsync);

            #endregion

            #region proxy

            this.useNoProxyRadioButton.IsChecked = true;
            this.useSystemProxySettingsCheckBox.IsChecked = settings.AutodetectProxy;
            this.useProxyCheckBox.IsChecked = settings.UseProxy;
            this.proxyHostTextBox.Text = settings.ProxyHost;
            this.proxyPortTextBox.Text = settings.ProxyPort.ToString();
            this.proxyUsernameTextBox.Text = settings.ProxyUsername;
            this.proxyPasswordBox.Password = settings.ProxyPassword;

            #endregion

            #region reminder

            this.remindToTrackCheckBox.IsChecked = settings.Reminder;
            this.remindToTrackIntervalTextBox.Text = settings.ReminderMinutes.ToString();
            this.reminderStartTimeTextBox.Text = settings.RemindStarts;
            this.reminderEndTimeTextBox.Text = settings.RemindEnds;

            this.remindOnMondayTextBox.IsChecked = settings.RemindMon;
            this.remindOnTuesdayTextBox.IsChecked = settings.RemindTue;
            this.remindOnWednesdayTextBox.IsChecked = settings.RemindWed;
            this.remindOnThursdayTextBox.IsChecked = settings.RemindThu;
            this.remindOnFridayTextBox.IsChecked = settings.RemindFri;
            this.remindOnSaturdayTextBox.IsChecked = settings.RemindSat;
            this.remindOnSundayTextBox.IsChecked = settings.RemindSun;

            #endregion

            #region auto tracker

            this.enableAutotrackerCheckbox.IsChecked = settings.Autotrack;

            #endregion

            #region global shortcuts

            trySetHotKey(
                Toggl.GetKeyShow,
                Toggl.GetKeyModifierShow,
                this.showHideShortcutRecorder
                );
            trySetHotKey(
                Toggl.GetKeyStart,
                Toggl.GetKeyModifierStart,
                this.continueStopShortcutRecorder
                );

            this.shortcutErrorText.Text = "";

            #endregion
        }

        private static async Task<bool?> IsRunOnStartupEnabled()
        {
#if MS_STORE
            return await RunOnStartup.IsRunOnStartupEnabled();
#else
            return Utils.GetLaunchOnStartupRegistry();
#endif
        }

        private async Task UpdateLaunchOnStartupCheckboxAsync()
        {
            var isEnabled = await IsRunOnStartupEnabled();
            this.TryBeginInvoke((bool? isRunOnStartupEnabled, CheckBox checkBox) =>
            {
                if (isRunOnStartupEnabled.HasValue == false)
                {
                    checkBox.Visibility = Visibility.Collapsed;
                }
                else
                {
                    checkBox.Visibility = Visibility.Visible;
                    checkBox.IsChecked = isRunOnStartupEnabled.Value;
                }
            }, isEnabled, launchOnStartupCheckBox);
        }

        private static void trySetHotKey(Func<string> getKeyCode, Func<ModifierKeys> getModifiers, ShortcutRecorder recorder)
        {
            try
            {
                var keyCode = getKeyCode();

                if (string.IsNullOrEmpty(keyCode))
                {
                    recorder.Reset(null);
                    return;
                }

                var modifiers = getModifiers();
                recorder.Reset(new Utils.KeyCombination(modifiers, keyCode));

            }
            catch (Exception e)
            {
                Toggl.Debug(string.Format("Could not load hotkey: {0}", e));
                recorder.Reset(null);
            }
        }

        private static string keyEventToString(Utils.KeyCombination shortcut)
        {
            var modifiers = shortcut.Modifiers;

            var res = "";
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Win))
            {
                res += "Win + ";
            }
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Control))
            {
                res += "Ctrl + ";
            }
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Shift))
            {
                res += "Shift + ";
            }
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Alt))
            {
                res += "Alt + ";
            }
            res += shortcut.KeyCode;
            return res;
        }

        private static bool isChecked(ToggleButton checkBox)
        {
            return checkBox.IsChecked ?? false;
        }

        private static ulong toULong(string text)
        {
            ulong.TryParse(text, out var ret);
            return ret;
        }

        private static long toLong(string text)
        {
            long.TryParse(text, out var ret);
            return ret;
        }

        #region saving

        private async void saveButtonClicked(object sender, RoutedEventArgs e)
        {
            try
            {
                this.isSaving = true;
                this.IsEnabled = false;

                using (Performance.Measure("saving settings"))
                {
                    var settings = this.createSettingsFromUI();

                    var success = await Task.Run(() => this.save(settings));

                    if (success)
                        this.Hide();
                }
            }
            finally
            {
                this.IsEnabled = true;
                this.isSaving = false;
            }
        }

        private async Task<bool> save(Settings settings)
        {
            using (Performance.Measure("saving global shortcuts"))
            {
                this.saveShortCuts();
            }

            Toggl.SetDefaultProject(settings.DefaultProject.ProjectID, settings.DefaultProject.TaskID);
            Toggl.SetKeepEndTimeFixed(settings.KeepEndTimeFixed);
#if MS_STORE
            await RunOnStartup.TrySetRunOnStartup(settings.LaunchOnStartup);
#else
            Utils.SaveLaunchOnStartupRegistry(settings.LaunchOnStartup);
#endif
            return Toggl.SetSettings(settings.TogglSettings);
        }

        private void saveShortCuts()
        {
            if (this.showHideShortcutRecorder.HasChanged)
                Utils.SetShortcutForShow(this.showHideShortcutRecorder.Shortcut);
            if (this.continueStopShortcutRecorder.HasChanged)
                Utils.SetShortcutForStart(this.continueStopShortcutRecorder.Shortcut);
        }

        private Settings createSettingsFromUI()
        {
            var settings = new Toggl.TogglSettingsView
            {
                #region general

                UseIdleDetection = isChecked(this.idleDetectionCheckBox),
                IdleMinutes = toLong(this.idleDetectionDurationTextBox.Text),

                RecordTimeline = isChecked(this.recordTimelineCheckBox),
                OnTop = isChecked(this.onTopCheckBox),

                Pomodoro = isChecked(this.enablePomodoroCheckBox),
                PomodoroMinutes = toLong(this.pomodoroTimerDuration.Text),

                PomodoroBreak = isChecked(this.enablePomodoroBreakCheckBox),
                PomodoroBreakMinutes = toLong(this.pomodoroBreakTimerDuration.Text),

                StopEntryOnShutdownSleep = isChecked(this.onStopEntryCheckBox),

                #endregion

                #region proxy

                AutodetectProxy = isChecked(this.useSystemProxySettingsCheckBox),
                UseProxy = isChecked(this.useProxyCheckBox),
                ProxyHost = this.proxyHostTextBox.Text,
                ProxyPort = toULong(this.proxyPortTextBox.Text),
                ProxyUsername = this.proxyUsernameTextBox.Text,
                ProxyPassword = this.proxyPasswordBox.Password,

                #endregion

                #region auto tracker

                Autotrack = isChecked(this.enableAutotrackerCheckbox),

                #endregion

                #region reminder

                Reminder = isChecked(this.remindToTrackCheckBox),
                ReminderMinutes = toLong(this.remindToTrackIntervalTextBox.Text),
                RemindStarts = this.reminderStartTimeTextBox.Text,
                RemindEnds = this.reminderEndTimeTextBox.Text,

                RemindMon = isChecked(this.remindOnMondayTextBox),
                RemindTue = isChecked(this.remindOnTuesdayTextBox),
                RemindWed = isChecked(this.remindOnWednesdayTextBox),
                RemindThu = isChecked(this.remindOnThursdayTextBox),
                RemindFri = isChecked(this.remindOnFridayTextBox),
                RemindSat = isChecked(this.remindOnSaturdayTextBox),
                RemindSun = isChecked(this.remindOnSundayTextBox),

                #endregion
            };

            return new Settings
            {
                TogglSettings = settings,
                DefaultProject = this.selectedDefaultProject,
                KeepEndTimeFixed = isChecked(this.keepEndTimeFixedCheckbox),
                LaunchOnStartup = isChecked(this.launchOnStartupCheckBox)
            };
        }

        class Settings
        {
            public Toggl.TogglSettingsView TogglSettings { get; set; }
            public Toggl.TogglAutocompleteView DefaultProject { get; set; }
            public bool KeepEndTimeFixed { get; set; }
            public bool LaunchOnStartup { get; set; }
        }

        #endregion

        #region shortcuts

        private class ShortcutRecorder
        {
            private static readonly Dictionary<Utils.KeyCombination, string> knownShortcuts =
                new Dictionary<Utils.KeyCombination, string>
            {
                { new Utils.KeyCombination(ModifierKeys.Control, "N"), "New time entry" },
                { new Utils.KeyCombination(ModifierKeys.Control, "O"), "Continue last entry" },
                { new Utils.KeyCombination(ModifierKeys.Control, "S"), "Stop time entry" },
                { new Utils.KeyCombination(ModifierKeys.Control, "W"), "Hide Toggl Desktop" },
                { new Utils.KeyCombination(ModifierKeys.Control, "R"), "Sync" },
                { new Utils.KeyCombination(ModifierKeys.Control, "E"), "Edit time entry" },
                { new Utils.KeyCombination(ModifierKeys.Control, "D"), "Toggle manual mode" },
                { new Utils.KeyCombination(ModifierKeys.Control, "V"), "New time entry from clipboard" },
            };

            private IReadOnlyList<ShortcutRecorder> allSettableShortcuts;

            private readonly Button button;
            private readonly TextBlock errorText;
            private bool recording;
            private ModifierKeys activatedModifiers;

            public string ShortcutName { get; private set; }

            public bool HasChanged { get; private set; }
            public Utils.KeyCombination? Shortcut { get; private set; }


            public ShortcutRecorder(Button button, Button clearButton, TextBlock errorText, string shortcutName)
            {
                this.button = button;
                this.errorText = errorText;
                this.ShortcutName = shortcutName;
                button.Click += (sender, args) => this.startRecording();
                button.PreviewKeyDown += this.onKeyDown;
                button.PreviewKeyUp += this.onKeyUp;
                button.LostKeyboardFocus += (sender, args) =>
                {
                    if (this.recording)
                        this.Reset();
                };
                clearButton.Click += (sender, args) => this.clear();
                this.button.Content = recordButtonIdleText;
            }

            public void SetShortcutList(IReadOnlyList<ShortcutRecorder> shortcutRecorders)
            {
                if (this.allSettableShortcuts != null)
                    throw new Exception("Cannot set known shortcuts more than once.");

                this.allSettableShortcuts = shortcutRecorders;
            }

            private void clear()
            {
                this.Reset();
                this.Shortcut = null;
                this.HasChanged = true;
                this.button.Content = recordButtonIdleText;
            }

            private void startRecording()
            {
                this.recording = true;
                this.button.Content = recordButtonRecordingText;
                this.activatedModifiers = ModifierKeys.None;
                this.errorText.Text = "";
            }

            private const ModifierKeys requiredModifiersUnion =
                ModifierKeys.Alt | ModifierKeys.Control | ModifierKeys.Win;

            private void onKeyDown(object sender, KeyEventArgs e)
            {
                // this method is needed in addition to the one below,
                // to detect the WIN keys (they are not included in Keyboard.Modifiers)

                if (!this.recording)
                    return;

                var key = (e.Key == Key.System ? e.SystemKey : e.Key);

                this.checkModifiers(key);
            }

            private void onKeyUp(object sender, KeyEventArgs e)
            {
                if (!this.recording)
                    return;

                var key = (e.Key == Key.System ? e.SystemKey : e.Key);

                // ignore modifier key releases
                if (this.checkModifiers(key))
                    return;

                var mods = this.activatedModifiers;

                if ((mods & requiredModifiersUnion) == ModifierKeys.None)
                {
                    if (key == Key.Enter || key == Key.Space)
                    {
                        // this happens when user starts recoding with keyboard
                        return;
                    }

                    this.errorText.Text = "Shortcut must contain Alt, Ctrl, or Windows key.";

                    this.cancelRecording();
                    return;
                }

                this.cancelRecording();

                if (key == Key.None)
                {
                    this.errorText.Text = "Something went wrong. Please try again.";
                    return;
                }

                e.Handled = true;
                
                var keyString = key.ToString();

                var shortcut = new Utils.KeyCombination(mods, keyString);

                if (this.findCollision(shortcut))
                    return;

                this.setShortcut(shortcut);
            }

            private void setShortcut(Utils.KeyCombination shortcut)
            {
                this.Shortcut = shortcut;
                this.HasChanged = true;

                this.button.Content = keyEventToString(shortcut);
                this.errorText.Text = "";
            }

            private bool findCollision(Utils.KeyCombination shortcut)
            {
                string collision;

                if (!knownShortcuts.TryGetValue(shortcut, out collision)
                    && this.allSettableShortcuts != null)
                {
                    var collisionRecorder = this.allSettableShortcuts
                        .FirstOrDefault(r => r != this && r.Shortcut == shortcut);

                    if (collisionRecorder != null)
                    {
                        collision = collisionRecorder.ShortcutName;
                    }
                }

                if (collision == null)
                    return false;

                this.errorText.Text = string.Format(
                    "{0} already taken by: {1}",
                    keyEventToString(shortcut), collision
                    );

                return true;
            }

            private bool checkModifier(
                Key capturedKey, Key modifierLeft, Key modifierRight, ModifierKeys modifier)
            {
                if (capturedKey != modifierLeft && capturedKey != modifierRight)
                    return false;

                this.activatedModifiers |= modifier;
                return true;
            }


            private bool checkModifiers(Key key)
            {
                return this.checkModifier(key, Key.LeftShift, Key.RightShift, ModifierKeys.Shift)
                    || this.checkModifier(key, Key.LeftAlt, Key.RightAlt, ModifierKeys.Alt)
                    || this.checkModifier(key, Key.LeftCtrl, Key.RightCtrl, ModifierKeys.Control)
                    || this.checkModifier(key, Key.LWin, Key.RWin, ModifierKeys.Win);
            }

            private void cancelRecording()
            {
                if (this.Shortcut.HasValue)
                {
                    var shortcut = this.Shortcut.Value;
                    this.button.Content = keyEventToString(shortcut);
                }
                else
                {
                    this.button.Content = recordButtonIdleText;
                }
                this.recording = false;
            }

            public void Reset()
            {
                this.cancelRecording();
                this.HasChanged = false;
            }

            public void Reset(Utils.KeyCombination? shortcut)
            {
                this.Shortcut = shortcut;
                this.Reset();
            }

        }

        #endregion

        #region project auto completion

        private void defaultProjectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.defaultProjectTextBox.Text == "")
            {
                this.selectDefaultProject(null);
            }
            else if (!string.IsNullOrEmpty(this.selectedDefaultProject.ProjectLabel))
            {
                this.selectDefaultProject(this.selectedDefaultProject);
            }
        }

        private void defaultProjectAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asProjectItem = e as TimerItem;
            if (asProjectItem == null)
                return;

            var item = asProjectItem.Item;

            this.selectDefaultProject(item);
        }

        private void defaultProjectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            this.selectDefaultProject(null);
        }


        private void selectDefaultProject(Toggl.TogglAutocompleteView? item)
        {
            var project = item ?? default(Toggl.TogglAutocompleteView);
            this.selectedDefaultProject = project;
            this.defaultProjectTextBox.SetText(project.ProjectLabel, project.TaskLabel);
            this.defaultProjectColorCircle.Background = Utils.ProjectColorBrushFromString(project.ProjectColor);
            this.defaultProjectTextBox.CaretIndex = this.defaultProjectTextBox.Text.Length;
        }

        private void selectDefaultProjectFromSettings()
        {
            var projectID = Toggl.GetDefaultProjectId();
            var taskID = Toggl.GetDefaultTaskId();

            var project = default(Toggl.TogglAutocompleteView);

            if (this.knownProjects != null)
            {
                project = this.knownProjects
                    .FirstOrDefault(p => p.ProjectID == projectID && p.TaskID == taskID);
            }

            if (project.ProjectID != projectID || project.TaskID != taskID)
            {
                project = new Toggl.TogglAutocompleteView
                {
                    ProjectLabel = Toggl.GetDefaultProjectName(),
                    ProjectID = projectID,
                    TaskID = taskID,
                }; 
            }

            this.selectDefaultProject(project);
        }

        #endregion
    }
}
