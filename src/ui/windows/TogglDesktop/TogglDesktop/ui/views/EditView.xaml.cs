﻿using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using DynamicData.Binding;
using MahApps.Metro.Controls;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;
using TogglDesktop.Converters;
using TogglDesktop.Diagnostics;
using TogglDesktop.ViewModels;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;
using TextBox = System.Windows.Controls.TextBox;

namespace TogglDesktop
{
    public partial class EditView
    {
        private Toggl.TogglTimeEntryView timeEntry;
        private bool isInNewProjectMode = true;
        private List<Toggl.TogglAutocompleteView> projects;
        private List<Toggl.TogglGenericView> clients;
        private List<Toggl.TogglGenericView> workspaces;
        private ulong selectedWorkspaceId;
        private string selectedClientGUID;
        private ulong selectedClientId;
        private string selectedClientName;
        private bool isCreatingProject;
        private bool startDateSet = false;
        public EditView()
        {
            this.DataContext = this;
            this.InitializeComponent();

            Toggl.OnLogin += this.onLogin;
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnMinitimerAutocomplete += this.onMinitimerAutocomplete;
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;
            Toggl.OnClientSelect += this.onClientSelect;
            Toggl.OnTags += this.onTags;
            Toggl.OnWorkspaceSelect += this.onWorkspaceSelect;
        }

        private void onLogin(bool open, ulong userId)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userId))
                return;

            this.timeEntry = new Toggl.TogglTimeEntryView();
            this.projects = null;
            this.clients = null;
            this.workspaces = null;
        }

        #region helper methods

        private bool hasTimeEntry()
        {
            return this.timeEntry.GUID != null;
        }

        #endregion

        #region fill with data

        #region from time entry

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView timeEntry, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, timeEntry, focusedFieldName))
                return;

            using (Performance.Measure("filling edit view from OnTimeEntryEditor"))
            {
                this.startDateSet = false;
                if (timeEntry.Locked)
                {
                    open = true;
                    this.contentGrid.IsEnabled = false;
                    this.contentGrid.Opacity = 0.75;
                }
                else
                {
                    this.contentGrid.IsEnabled = true;
                    this.contentGrid.Opacity = 1;
                }

                var keepNewProjectModeOpen =
                    !open
                    && this.isInNewProjectMode
                    && this.hasTimeEntry()
                    && this.timeEntry.GUID == timeEntry.GUID
                    && this.timeEntry.PID == timeEntry.PID
                    && this.timeEntry.WID == timeEntry.WID
                    && timeEntry.CanAddProjects;

                if (!keepNewProjectModeOpen && this.hasTimeEntry() && this.isInNewProjectMode)
                {
                    this.confirmNewProject();
                }

                var isDifferentTimeEntry = this.timeEntry.GUID != timeEntry.GUID;

                this.timeEntry = timeEntry;

                var isCurrentlyRunning = timeEntry.DurationInSeconds < 0;

                this.endTimeTextBox.IsEnabled = !isCurrentlyRunning;
                this.startDatePicker.IsEnabled = !isCurrentlyRunning;
                this.endDatePicker.IsEnabled = !isCurrentlyRunning;

                var startDateTime = Toggl.DateTimeFromUnix(timeEntry.Started);
                var endDateTime = Toggl.DateTimeFromUnix(timeEntry.Ended);

                setText(this.descriptionTextBox, timeEntry.Description, open);

                // Start:
                setTime(this.startTimeTextBox, timeEntry.StartTimeString, open);
                this.startTimeTextBox.ToolTip = startDateTime.ToString("T", CultureInfo.CurrentCulture);
                this.startDatePicker.SelectedDate = startDateTime;

                // End:
                setTime(this.endTimeTextBox, timeEntry.EndTimeString, open);
                this.endTimeTextBox.ToolTip = endDateTime.ToString("T", CultureInfo.CurrentCulture);
                this.endDatePicker.SelectedDate = endDateTime;

                // Duration:
                setTime(this.durationTextBox, timeEntry.Duration, open);
                

                if (isDifferentTimeEntry)
                {
                    this.clearUndoHistory();
                }

                if (isCurrentlyRunning)
                {
                    this.endDatePicker.SelectedDate = DateTime.Now;
                    this.endTimeTextBox.Text = "";
                }

                this.billableCheckBox.ShowOnlyIf(timeEntry.CanSeeBillable);
                this.billableCheckBox.IsChecked = timeEntry.Billable;

                if (open || !this.tagList.IsKeyboardFocusWithin)
                {
                    this.tagList.Clear(open);
                    if (timeEntry.Tags != null)
                        this.tagList.AddTags(timeEntry.Tags.Split(new[] {Toggl.TagSeparator},
                            StringSplitOptions.RemoveEmptyEntries));
                }

                if (!keepNewProjectModeOpen)
                {
                    if (this.isInNewProjectMode)
                    {
                        this.createProjectPopup.IsOpen = false;
                        this.disableNewProjectMode();
                    }

                    this.selectedProjectColorCircle.Background = Utils.AdaptedProjectColorBrushFromString(timeEntry.Color);

                    setText(this.projectTextBox, timeEntry.ProjectLabel, timeEntry.TaskLabel, open);
                    projectTextBox.DataContext = timeEntry.ToProjectLabelViewModel();

                    setText(this.clientTextBox, timeEntry.ClientLabel, open);

                    this.selectedWorkspaceId = timeEntry.WID;
                    this.reloadWorkspaceClients(timeEntry.WID);

                    this.projectAutoComplete.ActionButtonText =
                        timeEntry.CanAddProjects
                            ? "Create a new project"
                            : string.Empty;
                }
                this.startDateSet = true;
            }
        }

        #region helpers

        private static void setText(ProjectTextBox textBox, string project, string task, bool evenIfFocused)
        {
            if (evenIfFocused || !textBox.IsKeyboardFocused)
            {
                textBox.SetText(project, task);
            }
        }

        private static void setText(ExtendedTextBox textBox, string text, bool evenIfFocused)
        {
            if (evenIfFocused || !textBox.IsKeyboardFocused)
            {
                textBox.SetText(text);
            }
        }

        /// <summary>
        /// When <paramref name="textBox"/> is not focused, or has an empty <see cref="FrameworkElement.Tag"/> property value, or when <paramref name="evenIfFocused"/> is <see langword="true"/>, this method stores <paramref name="time"/> into <paramref name="textBox"/>'s <see cref="FrameworkElement.Tag"/> property.<br/>
        /// Otherwise, this method does nothing.
        /// </summary>
        private static void setTime(ExtendedTextBox textBox, string time, bool evenIfFocused)
        {
            if (evenIfFocused
                || !textBox.IsKeyboardFocused
                || textBox.Tag == null
                || textBox.Text == (textBox.Tag as string))
            {
                textBox.SetText(time);
                textBox.Tag = time;
            }
        }

        #endregion

        #endregion

        #region duration auto update

        private void durationUpdateTimerTick(object sender, string durationText)
        {
            if (this.TryBeginInvoke(durationUpdateTimerTick, sender, durationText))
                return;

            if (!this.hasTimeEntry() || this.timeEntry.DurationInSeconds >= 0)
                return;

            if (this.durationTextBox.Tag != null)
            {
                if (this.durationTextBox.Text != (this.durationTextBox.Tag as string)
                    || this.durationTextBox.SelectedText != "")
                    return;
            }

            var caret = this.durationTextBox.CaretIndex;

            this.durationTextBox.SetText(durationText);
            this.durationTextBox.Tag = durationText;

            this.durationTextBox.CaretIndex = caret;
        }

        #endregion

        #region auto completion

        private void onMinitimerAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onMinitimerAutocomplete, list))
                return;

            using (Performance.Measure("building edit view entry auto complete controller, {0} items", list.Count))
            {
                this.descriptionAutoComplete.SetController(AutoCompleteControllersFactory.ForTimer(list));
            }
        }

        private void onProjectAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onProjectAutocomplete, list))
                return;

            this.projects = list;

            using (Performance.Measure("building edit view project auto complete controller, {0} items", this.projects.Count))
            {
                this.projectAutoComplete.SetController(AutoCompleteControllersFactory.ForProjects(list));
            }
        }

        private void onClientSelect(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onClientSelect, list))
                return;

            this.clients = list;

            using (Performance.Measure("building edit view client auto complete controller, {0} items", this.clients.Count))
            {
                this.clientAutoComplete.SetController(AutoCompleteControllersFactory.ForClients(list));
            }
        }

        private void onTags(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onTags, list))
                return;

            using (Performance.Measure("building edit view tags auto complete controller, {0} items", list.Count))
            {
                this.tagList.SetKnownTags(list.Select(m => m.Name));
            }
        }

        private void onWorkspaceSelect(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onWorkspaceSelect, list))
                return;

            this.workspaces = list;

            using (Performance.Measure("building edit view workspace auto complete controller, {0} items", list.Count))
            {
                this.workspaceComboBox.ItemsSource = list;
                this.workspaceComboBox.SelectedIndex = workspaces.FindIndex(ws => ws.ID == selectedWorkspaceId);
                this.workspaceComboBox.ShowOnlyIf(list.Count > 1);
            }
        }

        #endregion

        #endregion

        #region change data

        #region time and date

        private void startTimeTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.saveStartTimeIfChanged();
        }

        private void endTimeTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.saveEndTimeIfChanged();
        }

        private void durationTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.saveDurationIfChanged();
        }

        private void startTimeTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.saveStartTimeIfChanged();
                e.Handled = true;
            }
        }

        private void endTimeTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.saveEndTimeIfChanged();
                e.Handled = true;
            }
        }

        private void durationTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.saveDurationIfChanged();
                e.Handled = true;
            }
        }

        private void saveStartTimeIfChanged()
        {
            this.setTimeEntryTimeIfChanged(this.startTimeTextBox, Toggl.SetTimeEntryStart, "start time");
        }
        private void saveEndTimeIfChanged()
        {
            this.setTimeEntryEndTimestampIfChanged();
        }
        private void saveDurationIfChanged()
        {
            this.setTimeEntryTimeIfChanged(this.durationTextBox, Toggl.SetTimeEntryDuration, "duration");
        }

        private void setTimeEntryTimeIfChanged(TextBox textBox, Func<string, string, bool> apiCall, string timeType)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply " + timeType + " change: No time entry.");
                return;
            }

            var before = textBox.Tag as string; // e.g. "06:00" or "6:00 AM" etc
            var now = textBox.Text; // e.g. "06:01" or "6:01 AM" etc
            if (before == now)
                return;

            textBox.Tag = null;

            apiCall(this.timeEntry.GUID, now);
        }

        private void setTimeEntryEndTimestampIfChanged()
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply change: No time entry.");
                return;
            }

            if (!this.endDatePicker.SelectedDate.HasValue)
            {
               Console.WriteLine("Cannot apply change: No end date value.");
               return;
            }

            DateTime nowEnd;
            DateTime previousEnd;
            {
                DateTime nowEndDate      = this.endDatePicker.SelectedDate.Value.Date;
                DateTime previousEndDate = ( this.endDatePicker.Tag as DateTime? ?? DateTime.MinValue ).Date;
            
                TimeSpan nowEndTimeOfDay;
                TimeSpan previousEndTimeOfDay;
                {
                    if (!DateTime.TryParseExact(this.endTimeTextBox.Text, format: "t", CultureInfo.CurrentCulture, DateTimeStyles.NoCurrentDateDefault | DateTimeStyles.AssumeLocal, out DateTime nowEndTimeOfDayAsDate))
                    {
                        Console.WriteLine("Cannot apply change: Invalid end time textbox value.");
                        return;
                    }

                    if (!DateTime.TryParseExact(this.endTimeTextBox.Tag as string, format: "t", CultureInfo.CurrentCulture, DateTimeStyles.NoCurrentDateDefault | DateTimeStyles.AssumeLocal, out DateTime previousEndTimeOfDayAsDate))
                    {
                        previousEndTimeOfDayAsDate = DateTime.MinValue;
                    }

                    nowEndTimeOfDay      = nowEndTimeOfDayAsDate.TimeOfDay;
                    previousEndTimeOfDay = previousEndTimeOfDayAsDate.TimeOfDay;
                }

                nowEnd      = nowEndDate     .Add(nowEndTimeOfDay);
                previousEnd = previousEndDate.Add(previousEndTimeOfDay);
            }

            long nowEndUnix      = Toggl.UnixFromDateTime(nowEnd);
            long previousEndUnix = Toggl.UnixFromDateTime(previousEnd);
            if( nowEndUnix == previousEndUnix )
            {
                Console.WriteLine("Cannot apply change: No change detected.");
                return;
            }

            this.endTimeTextBox.Tag = null;

            Toggl.SetTimeEntryEndTimeStamp(this.timeEntry.GUID, nowEndUnix);
        }

        #region datepicker

        private void saveDate() 
        { 
            if (!this.hasTimeEntry()) 
            { 
                Console.WriteLine("Cannot apply date change: No time entry."); 
                return; 
            } 
            if (!this.startDatePicker.SelectedDate.HasValue) 
            { 
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(this.timeEntry.Started); 
                return; 
            } 
 
            DateTime currentDate = Toggl.DateTimeFromUnix(timeEntry.Started); 
 
            if (!currentDate.Equals(this.startDatePicker.SelectedDate.Value)) 
            { 
                currentDate = this.startDatePicker.SelectedDate.Value; 
                Toggl.SetTimeEntryDate(this.timeEntry.GUID, currentDate); 
            } 
        } 

        private void startDatePicker_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!startDatePicker.IsKeyboardFocusWithin)
            {
                if (this.startDateSet)
                {
                    this.saveDate();
                }
            }
        }

        private void endDatePicker_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!endDatePicker.IsKeyboardFocusWithin)
            {
                this.setTimeEntryEndTimestampIfChanged();
            }
        }

        #endregion

        #endregion

        #region description

        private void descriptionAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            var asTimerItem = e as IModelItem<Toggl.TogglAutocompleteView>;
            if (asTimerItem == null)
                return;

            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply description change: No time entry.");
                return;
            }

            var item = asTimerItem.Model;

            this.descriptionTextBox.SetText(item.Description);
            this.descriptionTextBox.CaretIndex = this.descriptionTextBox.Text.Length;

            Toggl.SetTimeEntryDescription(this.timeEntry.GUID, item.Description);

            if (item.ProjectID != 0)
            {
                Toggl.SetTimeEntryProject(this.timeEntry.GUID, item.TaskID, item.ProjectID, "");
            }

            this.billableCheckBox.IsChecked = item.Billable;
            this.billableCheckBox_OnClick(null, null);

            if (!string.IsNullOrEmpty(item.Tags) && this.tagList.TagCount == 0)
            {
                this.tagList.Clear(true);
                if (item.Tags != null)
                    this.tagList.AddTags(item.Tags.Split(new[] { Toggl.TagSeparator },
                        StringSplitOptions.RemoveEmptyEntries));
                this.saveTags();
            }
        }

        private void descriptionAutoComplete_OnConfirmWithoutCompletion(object sender, string text)
        {
            this.setDescriptionIfChanged(text);
        }

        private void descriptionTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.setDescriptionIfChanged(this.descriptionTextBox.Text);
        }

        private void descriptionTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.setDescriptionIfChanged(this.descriptionTextBox.Text);
                e.Handled = true;
            }
        }

        private void setDescriptionIfChanged(string text)
        {
            if (text == null || text == this.timeEntry.Description)
                return;

            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply description change: No time entry.");
                return;
            }

            Toggl.SetTimeEntryDescription(this.timeEntry.GUID, text);
        }

        #endregion

        #region project

        private void projectAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            switch (e)
            {
                case IModelItem<Toggl.TogglAutocompleteView> projectItem:
                {
                    var item = projectItem.Model;
                    this.setProjectIfDifferent(item);
                    break;
                }
                default:
                    return;
            }
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            if (this.projectTextBox.Text == "")
            {
                this.setProjectIfDifferent(default);
            }
            else
            {
                // TODO: reset project? add new? switch to 'add project mode'?
            }
        }

        private void projectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if(this.isInNewProjectMode)
                return;

            if (this.projectTextBox.Text == "")
            {
                this.setProjectIfDifferent(default);
            }
            else
            {
                // TODO: if only one entry is left in auto complete box, should it be selected?

                this.projectTextBox.SetText(this.timeEntry.ProjectLabel, this.timeEntry.TaskLabel);
                projectTextBox.DataContext = timeEntry.ToProjectLabelViewModel();
            }

        }

        private void setProjectIfDifferent(Toggl.TogglAutocompleteView autoCompleteItem)
        {
            this.projectTextBox.SetText(autoCompleteItem.ProjectLabel ?? "", autoCompleteItem.TaskLabel ?? "");
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
            if (autoCompleteItem.ProjectID == this.timeEntry.PID && autoCompleteItem.TaskID == this.timeEntry.TID)
                return;
            this.projectTextBox.DataContext = autoCompleteItem.ProjectID == 0 ? null : autoCompleteItem.ToProjectLabelViewModel();
            this.selectedProjectColorCircle.Background = Utils.AdaptedProjectColorBrushFromString(autoCompleteItem.ProjectColor);
            Toggl.SetTimeEntryProject(this.timeEntry.GUID, autoCompleteItem.TaskID, autoCompleteItem.ProjectID, "");
        }

        #endregion

        #region new project mode

        private void enableNewProjectMode()
        {
            this.isProjectPrivateCheckBox.IsChecked = true;
            this.isProjectPrivateCheckBox.Visibility = Visibility.Visible;
            this.showClientArea();

            this.projectTextBox.SetText("", "");
            this.projectTextBox.DataContext = null;
            this.newProjectTextBox.Clear();
            this.projectAutoComplete.IsEnabled = false;
            this.newProjectTextBox.Focus();
            this.workspaceComboBox.SelectedIndex = workspaces.FindIndex(ws => ws.ID == selectedWorkspaceId);

            this.projectColorSelector.SelectRandom();
            this.projectTextBox.SetValue(TextBoxHelper.WatermarkProperty, "Add project");
            this.selectedProjectColorCircle.Background = Utils.AdaptedProjectColorBrushFromString("#999999");

            this.isInNewProjectMode = true;
        }

        private void disableNewProjectMode()
        {
            this.resetToSavedClient();
            this.isProjectPrivateCheckBox.Visibility = Visibility.Collapsed;
            this.clientAutoComplete.IsOpen = false;

            this.projectTextBox.SetText(this.timeEntry.ProjectLabel, this.timeEntry.TaskLabel);
            projectTextBox.DataContext = timeEntry.ToProjectLabelViewModel();
            this.projectAutoComplete.IsEnabled = true;
            this.projectTextBox.Focus();
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
            this.selectedProjectColorCircle.Background = Utils.AdaptedProjectColorBrushFromString(timeEntry.Color);
            this.projectTextBox.SetValue(TextBoxHelper.WatermarkProperty, "Select project");

            this.isInNewProjectMode = false;
        }

        private void projectTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.isInNewProjectMode)
                return;

            switch (e.Key)
            {
                case Key.Escape:
                    {
                        this.createProjectPopup.IsOpen = false;
                        e.Handled = true;
                        break;
                    }
                case Key.Enter:
                    {
                        this.confirmNewProject();
                        e.Handled = true;
                        break;
                    }
            }
        }

        private void confirmNewProject()
        {
            if (this.isCreatingProject)
                return;

            if (this.tryCreatingNewProject(this.newProjectTextBox.Text, this.projectColorSelector.SelectedColor))
            {
                this.createProjectPopup.IsOpen = false;
            }
        }

        private bool tryCreatingNewProject(string text, string color)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot add new project: No time entry.");
                return false;
            }

            this.isCreatingProject = true;

            var ret = Toggl.AddProject(
                this.timeEntry.GUID, this.selectedWorkspaceId,
                this.selectedClientId, this.selectedClientGUID,
                text, isProjectPrivateCheckBox.IsChecked.GetValueOrDefault(true), color) != null;

            this.isCreatingProject = false;

            return ret;
        }

        private void projectSaveButton_Click(object sender, RoutedEventArgs e)
        {
            this.confirmNewProject();
        }

        private void projectCancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.createProjectPopup.IsOpen = false;
        }

        #endregion

        #region client

        private void showClientArea()
        {
            this.clientTextBox.Text = "";
            this.selectedClientName = "";
            this.selectedClientGUID = "";
            this.selectedClientId = 0;
            this.clientAutoComplete.IsOpen = false;
        }

        private void clientAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            var asClientItem = e as IModelItem<Toggl.TogglGenericView>;
            if (asClientItem == null)
                return;

            var item = asClientItem.Model;

            this.selectClient(item);

            if (this.newProjectTextBox.Text.IsNullOrEmpty())
            {
                this.newProjectTextBox.Focus();
            }
        }

        private void selectClient(Toggl.TogglGenericView item)
        {
            this.selectedClientGUID = item.GUID;
            this.selectedClientId = item.ID;
            this.selectedClientName = item.Name;
            this.clientTextBox.SetText(item.Name);

            if (item.WID != 0 && item.WID != this.selectedWorkspaceId)
            {
                this.selectedWorkspaceId = item.WID;
                this.workspaceComboBox.SelectedIndex = workspaces.FindIndex(ws => ws.ID == selectedWorkspaceId);
            }
        }

        private void reloadWorkspaceClients(ulong workspace_id)
        {
            var list = this.clients.Where(c => c.WID == workspace_id).ToList();
            using (Performance.Measure("building Filtered edit view client auto complete controller, {0} items", this.clients.Count))
            {
                this.clientAutoComplete.SetController(AutoCompleteControllersFactory.ForClients(list));
            }
        }

        private void clientAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.TogglGenericView());
            }
            else
            {
                this.confirmNewClient();
            }
        }

        private void clientTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (clientAutoComplete.HasKeyboardFocus())
            {
                return;
            }

            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.TogglGenericView());
            }
            else
            {
                // TODO: if only one entry is left in auto complete box, should it be selected?
                this.clientTextBox.SetText(this.selectedClientName);
            }

            this.clientAutoComplete.IsOpen = false;
        }

        #endregion

        #region new client mode

        private void resetToSavedClient()
        {
            this.clientTextBox.SetText(this.selectedClientName);
            this.clientTextBox.Focus();
            this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
        }

        private void confirmNewClient()
        {
            this.tryCreatingNewClient(this.clientTextBox.Text);
        }

        private bool tryCreatingNewClient(string text)
        {
            var clientGUID = Toggl.CreateClient(this.selectedWorkspaceId, text);

            Console.WriteLine("client guid: " + (clientGUID ?? "null"));

            if (string.IsNullOrEmpty(clientGUID))
                return false;

            this.selectedClientName = text;
            this.selectedClientGUID = clientGUID;
            this.selectedClientId = 0;

            this.resetToSavedClient();

            if (this.newProjectTextBox.Text.IsNullOrEmpty())
            {
                this.newProjectTextBox.Focus();
            }

            return true;
        }

        #endregion

        #region workspace

        private void selectWorkspace(Toggl.TogglGenericView item)
        {
            if (this.selectedWorkspaceId != item.ID)
            {
                this.reloadWorkspaceClients(item.ID);
                this.selectClient(new Toggl.TogglGenericView());
            }

            this.selectedWorkspaceId = item.ID;
            this.workspaceComboBox.SelectedIndex = workspaces.FindIndex(ws => ws.ID == selectedWorkspaceId);
        }

        private void workspaceTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.workspaceComboBox.SelectedIndex < 0)
            {
                this.workspaceComboBox.SelectedIndex = workspaces.FindIndex(ws => ws.ID == selectedWorkspaceId);
            }
        }
        #endregion

        #region tag list

        private void tagList_TagAdded(object sender, string e)
        {
            this.saveTags();
        }

        private void tagList_TagRemoved(object sender, string e)
        {
            this.saveTags();
        }

        private void saveTags()
        {
            Toggl.SetTimeEntryTags(this.timeEntry.GUID, this.tagList.Tags.ToList());
        }

        #endregion

        private void billableCheckBox_OnClick(object sender, RoutedEventArgs e)
        {
            Toggl.SetTimeEntryBillable(this.timeEntry.GUID, this.billableCheckBox.IsChecked ?? false);
        }

        #endregion

        #region variuos

        public void SetTimer(Timer timer)
        {
            timer.ViewModel.WhenValueChanged(x => x.DurationText).Subscribe(durationText => this.durationUpdateTimerTick(this, durationText));
        }

        public void FocusField(string focusedFieldName)
        {
            UIElement focus = null;
            switch (focusedFieldName)
            {
                case Toggl.Project:
                    focus = this.projectTextBox;
                    break;
                case Toggl.Duration:
                    focus = this.durationTextBox;
                    break;
                case Toggl.Description:
                    focus = this.descriptionTextBox;
                    break;
                default:
                    if (!this.IsKeyboardFocusWithin)
                        focus = this.descriptionTextBox;
                    break;
            }

            focus?.Focus();
        }

        private void deleteButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (this.timeEntry.ConfirmlessDelete())
            {
                Toggl.DeleteTimeEntry(this.timeEntry.GUID);
                return;
            }
            Toggl.AskToDeleteEntry(this.timeEntry.GUID);
        }

        private void clearUndoHistory()
        {
            descriptionTextBox.ClearUndoHistory();
            startTimeTextBox.ClearUndoHistory();
            endTimeTextBox.ClearUndoHistory();
            durationTextBox.ClearUndoHistory();
            tagList.ClearUndoHistory();
        }

        #endregion

        private void ProjectAutoComplete_OnActionButtonClick(object sender, RoutedEventArgs e)
        {
            createProjectPopup.IsOpen = true;
        }

        private void CreateProjectPopup_OnClosed(object sender, EventArgs e)
        {
            disableNewProjectMode();
        }

        private void CreateProjectPopup_OnOpened(object sender, EventArgs e)
        {
            projectAutoComplete.IsOpen = false;
            enableNewProjectMode();
        }

        private void ClientAutoComplete_OnActionButtonClick(object sender, RoutedEventArgs e)
        {
            this.confirmNewClient();
        }

        private void ClientAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            if (clientAutoComplete.IsOpen == false)
            {
                if (clientTextBox.Text != this.selectedClientName)
                {
                    this.clientTextBox.SetText(this.selectedClientName);
                }
            }
        }

        private void WorkspaceComboBox_OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (workspaceComboBox.SelectedIndex < 0) return;
            var selectedWorkspace = workspaces[workspaceComboBox.SelectedIndex];
            selectWorkspace(selectedWorkspace);
        }
    }
}