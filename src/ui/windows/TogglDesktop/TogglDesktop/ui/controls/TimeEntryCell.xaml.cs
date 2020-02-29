using System.Windows;
using System.Windows.Input;
using ReactiveUI;
using TogglDesktop.Diagnostics;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class TimeEntryCell : IViewFor<TimeEntryCellViewModel>
    {
        public TimeEntryCellViewModel ViewModel
        {
            get => (TimeEntryCellViewModel)DataContext;
            set => DataContext = value;
        }

        object IViewFor.ViewModel
        {
            get => ViewModel;
            set => ViewModel = (TimeEntryCellViewModel) value;
        }

        public bool IsDummy
        {
            set
            {
                if (value == false)
                    return;

                this.IsEnabled = false;
            }
        }

        public TimeEntryCell()
        {
            this.InitializeComponent();
            ViewModel = new TimeEntryCellViewModel();
        }

        public void Display(Toggl.TogglTimeEntryView item)
        {
            ViewModel.Guid = item.GUID;
            ViewModel.IsGroup = item.Group;
            if (ViewModel.IsGroup)
            {
                ViewModel.IsGroupExpanded = item.GroupOpen;
                ViewModel.GroupName = item.GroupName;
                ViewModel.GroupItemCount = item.GroupItemCount;
            }

            ViewModel.IsSubItem = !item.Group && item.GroupOpen;
            ViewModel.DurationInSeconds = item.DurationInSeconds;
            ViewModel.TimeEntryLabel = item.ToTimeEntryLabelViewModel();
            timeEntryInfoPanel.SetTimeEntry(item);

            this.unsyncedIcon.ShowOnlyIf(item.Unsynced);
            this.lockedIcon.ShowOnlyIf(item.Locked);
        }

        #region open edit window event handlers

        private void labelDuration_MouseDown(object sender, MouseButtonEventArgs e)
        {
            this.openEditView(e, Toggl.Duration);
        }

        private void labelDescription_MouseDown(object sender, MouseButtonEventArgs e)
        {
            this.openEditView(e, Toggl.Description);
        }

        private void labelProject_MouseDown(object sender, MouseButtonEventArgs e)
        {
            this.openEditView(e, Toggl.Project);
        }

        private void entry_MouseDown(object sender, MouseButtonEventArgs e)
        {
            this.openEditView(e, "");
        }

        private void openEditView(MouseButtonEventArgs e, string focusedField)
        {
            if (ViewModel.IsGroup)
            {
                Toggl.ToggleEntriesGroup(ViewModel.GroupName);
                e.Handled = true;
                return;
            }
            using (Performance.Measure("opening edit view from cell, focussing " + focusedField))
            {
                Toggl.Edit(ViewModel.Guid, false, focusedField);
            }
            e.Handled = true;
        }

        #endregion

        private void buttonContinue_Click(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("continuing time entry from cell"))
            {
                Toggl.Continue(ViewModel.Guid);
            }
        }
    }
}
