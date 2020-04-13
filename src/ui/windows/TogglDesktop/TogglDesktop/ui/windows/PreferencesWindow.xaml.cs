﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using MahApps.Metro.Controls;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;
using TogglDesktop.Diagnostics;
using TogglDesktop.Theming;
using TogglDesktop.ViewModels;
using DayOfWeek = System.DayOfWeek;

#if MS_STORE
using TogglDesktop.Services.Win10;
#endif
namespace TogglDesktop
{
    partial class PreferencesWindow
    {
        public PreferencesWindowViewModel ViewModel
        {
            get => (PreferencesWindowViewModel)DataContext;
            private set => DataContext = value;
        }

        private bool isSaving;

        private Toggl.TogglAutocompleteView selectedDefaultProject;
        private List<Toggl.TogglAutocompleteView> knownProjects;
        private DayOfWeek lastBeginningOfWeek = DayOfWeek.Monday;

        public PreferencesWindow()
        {
            this.InitializeComponent();
            ViewModel = new PreferencesWindowViewModel(MessageBox.Show(this), this.Close);

            Toggl.OnSettings += this.onSettings;
            Toggl.OnLogin += this.onLogin;
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;

            this.Closing += OnClosing;
        }

        private void OnClosing(object sender, CancelEventArgs e)
        {
            ViewModel.ResetRecordedShortcuts();
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

            this.defaultProjectAutoComplete.SetController(AutoCompleteControllersFactory.ForProjects(list));
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
            this.keepDurationFixedCheckbox.IsChecked = !this.keepEndTimeFixedCheckbox.IsChecked;

            this.onStopEntryCheckBox.IsChecked = settings.StopEntryOnShutdownSleep;
            this.themeComboBox.SelectedIndex = settings.ColorTheme;

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

            this.remindDay1CheckBox.IsChecked = settings.RemindMon;
            this.remindDay2CheckBox.IsChecked = settings.RemindTue;
            this.remindDay3CheckBox.IsChecked = settings.RemindWed;
            this.remindDay4CheckBox.IsChecked = settings.RemindThu;
            this.remindDay5CheckBox.IsChecked = settings.RemindFri;
            this.remindDay6CheckBox.IsChecked = settings.RemindSat;
            this.remindDay7CheckBox.IsChecked = settings.RemindSun;

            this.SetBeginningOfWeek(Toggl.UserBeginningOfWeek());

            #endregion

            #region auto tracker

            this.enableAutotrackerCheckbox.IsChecked = settings.Autotrack;

            #endregion

            ViewModel.LoadShortcutsFromSettings();
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
            this.TryBeginInvoke((bool? isRunOnStartupEnabled, ToggleSwitch checkBox) =>
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

        private static bool isChecked(ToggleButton checkBox)
        {
            return checkBox.IsChecked ?? false;
        }

        private static bool isChecked(ToggleSwitch checkBox)
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
            ViewModel.ResetPropsWithErrorsToPreviousValues();

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
                Utils.SetShortcutForShow(settings.ShowHideToggl);
                Utils.SetShortcutForStart(settings.ContinueStopTimer);
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

        private Settings createSettingsFromUI()
        {
            var isRemindDayChecked = GetDaysOfWeekCheckboxes().Select(isChecked).ToArray();

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
                ColorTheme = (byte)this.themeComboBox.SelectedIndex,

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

                RemindMon = isRemindDayChecked[DayOfWeek.Monday.PositionRelativeTo(lastBeginningOfWeek)],
                RemindTue = isRemindDayChecked[DayOfWeek.Tuesday.PositionRelativeTo(lastBeginningOfWeek)],
                RemindWed = isRemindDayChecked[DayOfWeek.Wednesday.PositionRelativeTo(lastBeginningOfWeek)],
                RemindThu = isRemindDayChecked[DayOfWeek.Thursday.PositionRelativeTo(lastBeginningOfWeek)],
                RemindFri = isRemindDayChecked[DayOfWeek.Friday.PositionRelativeTo(lastBeginningOfWeek)],
                RemindSat = isRemindDayChecked[DayOfWeek.Saturday.PositionRelativeTo(lastBeginningOfWeek)],
                RemindSun = isRemindDayChecked[DayOfWeek.Sunday.PositionRelativeTo(lastBeginningOfWeek)],

                #endregion
            };

            return new Settings
            {
                TogglSettings = settings,
                DefaultProject = this.selectedDefaultProject,
                KeepEndTimeFixed = isChecked(this.keepEndTimeFixedCheckbox),
                LaunchOnStartup = isChecked(this.launchOnStartupCheckBox),
                ContinueStopTimer = ViewModel.GetContinueStopTimerIfChanged(),
                ShowHideToggl = ViewModel.GetShowHideTogglIfChanged()
            };
        }

        class Settings
        {
            public Toggl.TogglSettingsView TogglSettings { get; set; }
            public Toggl.TogglAutocompleteView DefaultProject { get; set; }
            public bool KeepEndTimeFixed { get; set; }
            public bool LaunchOnStartup { get; set; }
            public HotKey ShowHideToggl { get; set; }
            public HotKey ContinueStopTimer { get; set; }
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

        private void defaultProjectAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            var asProjectItem = e as IModelItem<Toggl.TogglAutocompleteView>;
            if (asProjectItem == null)
                return;

            var item = asProjectItem.Model;

            this.selectDefaultProject(item);
            this.defaultProjectTextBox.CaretIndex = this.defaultProjectTextBox.Text.Length;
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

        private void SetBeginningOfWeek(DayOfWeek beginningOfWeek)
        {
            var daysOfWeekCheckboxes = GetDaysOfWeekCheckboxes();
            var isDayOfWeekChecked = daysOfWeekCheckboxes.Select(c => c.IsChecked).ToArray();

            foreach (var dayOfWeek in DayOfWeekExtensions.DaysOfWeek())
            {
                var newDayPosition = dayOfWeek.PositionRelativeTo(beginningOfWeek);
                var oldDayPosition = dayOfWeek.PositionRelativeTo(DayOfWeek.Monday);
                daysOfWeekCheckboxes[newDayPosition].IsChecked = isDayOfWeekChecked[oldDayPosition];

                daysOfWeekCheckboxes[newDayPosition].Content = Enum.GetName(typeof(DayOfWeek), dayOfWeek);
            }

            lastBeginningOfWeek = beginningOfWeek;
        }

        private CheckBox[] GetDaysOfWeekCheckboxes() => new[]
        {
            remindDay1CheckBox,
            remindDay2CheckBox,
            remindDay3CheckBox,
            remindDay4CheckBox,
            remindDay5CheckBox,
            remindDay6CheckBox,
            remindDay7CheckBox,
        };
    }
}
