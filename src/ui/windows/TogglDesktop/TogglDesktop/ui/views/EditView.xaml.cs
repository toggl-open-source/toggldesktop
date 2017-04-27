using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;
using TextBox = System.Windows.Controls.TextBox;

namespace TogglDesktop
{
    public partial class EditView
    {
        private Toggl.TogglTimeEntryView timeEntry;
        private bool isInNewProjectMode = true;
        private bool isInNewClientMode = true;
        private List<Toggl.TogglAutocompleteView> projects;
        private List<Toggl.TogglGenericView> clients;
        private List<Toggl.TogglGenericView> workspaces;
        private ulong selectedWorkspaceId;
        private string selectedWorkspaceName;
        private string selectedClientGUID;
        private ulong selectedClientId;
        private string selectedClientName;
        private bool isCreatingProject;
        private bool dateSet = false;
        private bool confirmlessDelete = false;
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
                this.dateSet = false;
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

                this.timeEntry = timeEntry;

                this.confirmlessDelete = (timeEntry.Description.Length == 0
                    && timeEntry.DurationInSeconds < 15 && timeEntry.PID == 0);

                var isCurrentlyRunning = timeEntry.DurationInSeconds < 0;

                this.endTimeTextBox.IsEnabled = !isCurrentlyRunning;

                setText(this.descriptionTextBox, timeEntry.Description, open);
                setTime(this.durationTextBox, timeEntry.Duration, open);
                setTime(this.startTimeTextBox, timeEntry.StartTimeString, open);
                setTime(this.endTimeTextBox, timeEntry.EndTimeString, open);
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);

                if (isCurrentlyRunning)
                {
                    this.endTimeTextBox.Text = "";
                }

                this.billableCheckBox.ShowOnlyIf(timeEntry.CanSeeBillable);
                this.billableCheckBox.IsChecked = timeEntry.Billable;

                if (timeEntry.UpdatedAt > 0)
                {
                    var updatedAt = Toggl.DateTimeFromUnix(timeEntry.UpdatedAt);
                    this.lastUpdatedText.Text = "Last update " + updatedAt.ToShortDateString() + " at " +
                                                updatedAt.ToLongTimeString();
                    this.lastUpdatedText.Visibility = Visibility.Visible;
                }
                else
                {
                    this.lastUpdatedText.Visibility = Visibility.Collapsed;
                }

                if (open || !this.tagList.IsKeyboardFocusWithin)
                {
                    this.tagList.Clear(open);
                    if (timeEntry.Tags != null)
                        this.tagList.AddTags(timeEntry.Tags.Split(new[] {Toggl.TagSeparator},
                            StringSplitOptions.RemoveEmptyEntries));
                    this.updateTagListEmptyText();
                }

                if (!keepNewProjectModeOpen)
                {
                    if (this.isInNewProjectMode)
                        this.disableNewProjectMode();

                    this.projectColorSelector.SelectedColor = timeEntry.Color;

                    setText(this.projectTextBox, timeEntry.ProjectLabel, timeEntry.TaskLabel, open);
                    setText(this.clientTextBox, timeEntry.ClientLabel, open);

                    this.selectedWorkspaceId = timeEntry.WID;
                    this.selectedWorkspaceName = timeEntry.WorkspaceName;

                    if (timeEntry.CanAddProjects)
                    {
                        this.newProjectButton.Visibility = Visibility.Visible;
                        this.projectAddButtonColumn.Width = GridLength.Auto;
                        this.projectAddButtonColumn.SharedSizeGroup = "AddButtons";
                    }
                    else
                    {
                        this.newProjectButton.Visibility = Visibility.Hidden;
                        this.projectAddButtonColumn.Width = new GridLength(0);
                        this.projectAddButtonColumn.SharedSizeGroup = null;
                    }
                }
                this.dateSet = true;
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

        private void durationUpdateTimerTick(object sender, string s)
        {
            if (!this.hasTimeEntry() || this.timeEntry.DurationInSeconds >= 0)
                return;

            if (this.durationTextBox.Tag != null)
            {
                if (this.durationTextBox.Text != (this.durationTextBox.Tag as string)
                    || this.durationTextBox.SelectedText != "")
                    return;
            }

            var caret = this.durationTextBox.CaretIndex;

            this.durationTextBox.SetText(s);
            this.durationTextBox.Tag = s;

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
                this.descriptionAutoComplete.SetController(AutoCompleteControllers.ForTimer(list));
            }
        }

        private void onProjectAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onProjectAutocomplete, list))
                return;

            this.projects = list;

            using (Performance.Measure("building edit view project auto complete controller, {0} items", this.projects.Count))
            {
                this.projectAutoComplete.SetController(AutoCompleteControllers.ForProjects(list));
            }
        }

        private void onClientSelect(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onClientSelect, list))
                return;

            this.clients = list;

            using (Performance.Measure("building edit view client auto complete controller, {0} items", this.clients.Count))
            {
                this.clientAutoComplete.SetController(AutoCompleteControllers.ForClients(list));
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
                this.workspaceAutoComplete.SetController(AutoCompleteControllers.ForWorkspaces(list));
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
            this.setTimeEntryTimeIfChanged(this.endTimeTextBox, Toggl.SetTimeEntryEnd, "end time");
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

            var before = textBox.Tag as string;
            var now = textBox.Text;
            if (before == now)
                return;

            textBox.Tag = null;

            apiCall(this.timeEntry.GUID, now);
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
            try { 
                DateTime selected = Convert.ToDateTime(this.startDatePicker.Text);
           
                if (!currentDate.Date.Equals(selected.Date))
                {
                    currentDate = selected;
                    Toggl.SetTimeEntryDate(this.timeEntry.GUID, selected);
                }
            }
            catch (Exception e)
            {
                System.Console.WriteLine("Catched error: " + e.Message);
            }
        }

        private void startDatePicker_SelectedDateChanged(object sender, SelectionChangedEventArgs e)
        {
            if (this.dateSet)
            {
                this.saveDate();
            }
        }

        #endregion

        #endregion

        #region description

        private void descriptionAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asTimerItem = e as TimerItem;
            if (asTimerItem == null)
                return;

            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply description change: No time entry.");
                return;
            }

            var item = asTimerItem.Item;

            this.descriptionTextBox.SetText(item.Description);

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
                this.updateTagListEmptyText();
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

        private void projectAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asProjectItem = e as IProjectItem;
            if (asProjectItem == null)
                return;

            var item = asProjectItem.Item;

            this.setProjectIfDifferent(item.TaskID, item.ProjectID, item.ProjectLabel, item.TaskLabel, item.ProjectColor);
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            if (this.projectTextBox.Text == "")
            {
                this.setProjectIfDifferent(0, 0, "", "");
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
                this.setProjectIfDifferent(0, 0, "", "");
            }
            else
            {
                // TODO: if only one entry is left in auto complete box, should it be selected?

                this.projectTextBox.SetText(this.timeEntry.ProjectLabel, this.timeEntry.TaskLabel);
            }

        }

        private void setProjectIfDifferent(ulong taskId, ulong projectId, string projectName, string taskName, string projectColor = null)
        {
            if (projectId == this.timeEntry.PID && taskId == this.timeEntry.TID)
                return;
            this.projectTextBox.SetText(projectName, taskName);
            this.projectColorSelector.SelectedColor = projectColor;
            Toggl.SetTimeEntryProject(this.timeEntry.GUID, taskId, projectId, "");
        }

        private void newProjectButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.enableNewProjectMode();
        }
        private void newProjectCancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.disableNewProjectMode();
        }

        #endregion

        #region new project mode

        private void enableNewProjectMode()
        {
            this.showClientArea();

            this.projectTextBox.SetText("", "");
            this.projectTextBox.SetValue(Grid.ColumnSpanProperty, 2);
            this.projectAutoComplete.IsEnabled = false;
            this.projectDropDownButton.Visibility = Visibility.Hidden;
            this.newProjectButton.Visibility = Visibility.Hidden;
            this.newProjectCancelButton.Visibility = Visibility.Visible;
            this.projectSaveArea.Visibility = Visibility.Visible;
            this.projectTextBox.Focus();
            this.showWorkspaceArea();

            this.projectColorSelector.SelectRandom();
            this.projectColorSelector.IsEnabled = true;
            this.emptyProjectText.Text = "Add project";

            this.isInNewProjectMode = true;
        }

        private void disableNewProjectMode()
        {
            this.disableNewClientMode();
            this.hideClientArea();

            this.projectTextBox.SetText(this.timeEntry.ProjectLabel, this.timeEntry.TaskLabel);
            this.projectTextBox.SetValue(Grid.ColumnSpanProperty, 1);
            this.projectAutoComplete.IsEnabled = true;
            this.projectDropDownButton.Visibility = Visibility.Visible;
            this.newProjectButton.Visibility = Visibility.Visible;
            this.newProjectCancelButton.Visibility = Visibility.Hidden;
            this.projectSaveArea.Visibility = Visibility.Collapsed;
            this.projectTextBox.Focus();
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
            this.hideWorkspaceArea();

            this.projectColorSelector.SelectedColor = this.timeEntry.Color;
            this.projectColorSelector.IsEnabled = false;
            this.emptyProjectText.Text = "No project";

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
                        this.disableNewProjectMode();
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

            if (this.isInNewClientMode)
            {
                this.tryCreatingNewClient(this.clientTextBox.Text);
            }

            if (this.tryCreatingNewProject(this.projectTextBox.Text, this.projectColorSelector.SelectedColor))
            {
                this.disableNewProjectMode();
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
                text, false, color) != null;

            this.isCreatingProject = false;

            return ret;
        }

        private void projectSaveButton_Click(object sender, RoutedEventArgs e)
        {
            this.confirmNewProject();
        }

        private void projectCancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.disableNewProjectMode();
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
            this.clientArea.Visibility = Visibility.Visible;
        }

        private void hideClientArea()
        {
            this.clientArea.Visibility = Visibility.Collapsed;
            this.clientAutoComplete.IsOpen = false;
        }

        private void clientAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asClientItem = e as ModelItem;
            if (asClientItem == null)
                return;

            var item = asClientItem.Item;

            this.selectClient(item);

            this.projectTextBox.Focus();
        }

        private void selectClient(Toggl.TogglGenericView item)
        {
            this.selectedClientGUID = item.GUID;
            this.selectedClientId = item.ID;
            this.selectedClientName = item.Name;
            this.clientTextBox.SetText(item.Name);

            if (item.WID != 0)
            {
                this.selectedWorkspaceId = item.WID;
                this.selectedWorkspaceName = this.workspaces.First(ws => ws.ID == item.WID).Name;
                this.workspaceTextBox.SetText(this.selectedWorkspaceName);
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
                // TODO: reset client? add new? switch to 'add new client mode'?
            }
        }

        private void clientTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.isInNewClientMode)
                return;

            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.TogglGenericView());
            }
            else
            {
                // TODO: if only one entry is left in auto complete box, should it be selected?

                this.clientTextBox.SetText(this.selectedClientName);
            }

        }

        private void newClientButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.enableNewClientMode();
        }

        private void newClientCancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.disableNewClientMode();
            
            this.clientTextBox.SetText(this.timeEntry.ClientLabel);
            if (!string.IsNullOrEmpty(this.timeEntry.ClientLabel))
            {
                this.selectedWorkspaceId = this.timeEntry.WID;
                this.selectedWorkspaceName = this.timeEntry.WorkspaceName;
                this.workspaceTextBox.SetText(this.selectedWorkspaceName);
            }

            this.projectTextBox.Focus();
        }
        #endregion

        #region new client mode

        private void enableNewClientMode()
        {
            this.clientTextBox.SetText("");

            this.clientTextBox.SetValue(Grid.ColumnSpanProperty, 2);
            this.clientAutoComplete.IsEnabled = false;
            this.clientDropDownButton.Visibility = Visibility.Hidden;
            this.newClientButton.Visibility = Visibility.Hidden;
            this.newClientCancelButton.Visibility = Visibility.Visible;

            this.clientTextBox.Focus();

            this.emptyClientText.Text = "Add client";

            this.isInNewClientMode = true;
        }

        private void disableNewClientMode()
        {
            this.clientTextBox.SetText(this.selectedClientName);

            this.clientTextBox.SetValue(Grid.ColumnSpanProperty, 1);
            this.clientAutoComplete.IsEnabled = true;
            this.clientDropDownButton.Visibility = Visibility.Visible;
            this.newClientButton.Visibility = Visibility.Visible;
            this.newClientCancelButton.Visibility = Visibility.Hidden;

            this.clientTextBox.Focus();
            this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;

            this.emptyClientText.Text = "No client";

            this.isInNewClientMode = false;
        }

        private void clientTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.isInNewClientMode)
                return;

            switch (e.Key)
            {
                case Key.Escape:
                    {
                        this.disableNewClientMode();
                        e.Handled = true;
                        break;
                    }
                case Key.Enter:
                    {
                        this.confirmNewClient();
                        e.Handled = true;
                        break;
                    }
            }
        }

        private void confirmNewClient()
        {
            if (this.tryCreatingNewClient(this.clientTextBox.Text))
            {
                this.disableNewClientMode();
                this.projectTextBox.Focus();
            }
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

            this.disableNewClientMode();

            this.projectTextBox.Focus();

            return true;
        }

        #endregion

        #region workspace

        private void showWorkspaceArea()
        {
            this.workspaceTextBox.Text = this.selectedWorkspaceName;
            this.workspaceAutoComplete.IsOpen = false;
            this.workspaceArea.Visibility = Visibility.Visible;
        }

        private void hideWorkspaceArea()
        {
            this.workspaceArea.Visibility = Visibility.Collapsed;
            this.workspaceAutoComplete.IsOpen = false;
        }

        private void workspaceAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asWorkspaceItem = e as ModelItem;
            if (asWorkspaceItem == null)
                return;

            var item = asWorkspaceItem.Item;

            this.selectWorkspace(item);

            this.clientTextBox.Focus();
        }

        private void selectWorkspace(Toggl.TogglGenericView item)
        {
            if (this.selectedWorkspaceId != item.ID && !this.isInNewClientMode)
            {
                this.selectClient(new Toggl.TogglGenericView());
            }

            this.selectedWorkspaceId = item.ID;
            this.selectedWorkspaceName = item.Name;
            this.workspaceTextBox.SetText(item.Name);
        }

        private void workspaceAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            // TODO: reset client? add new? switch to 'add new client mode'?
        }

        private void workspaceTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.workspaceTextBox.SetText(this.selectedWorkspaceName);
        }
        #endregion

        #region tag list

        private void tagList_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot change tags: No time entry.");
                return;
            }

            this.updateTagListEmptyText();

            this.saveTags();
        }

        private void tagList_OnGotKeybardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.updateTagListEmptyText();
        }

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

        private void updateTagListEmptyText()
        {
            this.emptyTagListText.ShowOnlyIf(this.tagList.TagCount == 0 && !this.tagList.IsKeyboardFocusWithin);
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
            timer.RunningTimeEntrySecondPulse += this.durationUpdateTimerTick;
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Escape:
                {
                    this.close();
                    e.Handled = true;
                    return;
                }
            }
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

            if (focus != null)
            {
                focus.Focus();
            }
        }

        private void backButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.close();
        }

        private void close()
        {
            Toggl.ViewTimeEntryList();
        }

        public void EnsureSaved()
        {
            if (this.isInNewProjectMode)
            {
                this.confirmNewProject();
            }
        }

        private void deleteButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (this.confirmlessDelete)
            {
                Toggl.DeleteTimeEntry(this.timeEntry.GUID);
                return;
            }
            Toggl.AskToDeleteEntry(this.timeEntry.GUID);
        }

        #endregion

    }
}
