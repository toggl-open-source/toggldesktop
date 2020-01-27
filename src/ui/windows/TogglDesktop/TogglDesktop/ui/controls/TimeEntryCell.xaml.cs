using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
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
        private static readonly Color idleBackColor = Color.FromRgb(255, 255, 255);
        private static readonly Color hoverColor = Color.FromRgb(244, 244, 244);
        private static readonly Color subItemBackColor = Color.FromRgb(240, 240, 240);
        private static readonly Color defaultForegroundColor = Color.FromRgb(0, 0, 0);

        private Color entryHoverColor = hoverColor;

        public bool IsDummy
        {
            set
            {
                if (value == false)
                    return;

                this.IsEnabled = false;
            }
        }

        public bool SubItem
        {
            get { return (bool)this.GetValue(SubItemProperty); }
            set { this.SetValue(SubItemProperty, value); }
        }
        public static readonly DependencyProperty SubItemProperty = DependencyProperty
            .Register("SubItem", typeof(bool), typeof(TimeEntryCell), new FrameworkPropertyMetadata(false));

        public Color EntryBackColor
        {
            get { return (Color)this.GetValue(EntryBackColorProperty); }
            set { this.SetValue(EntryBackColorProperty, value); }
        }
        public static readonly DependencyProperty EntryBackColorProperty = DependencyProperty
            .Register("EntryBackColor", typeof(Color), typeof(TimeEntryCell), new FrameworkPropertyMetadata(idleBackColor));

        public TimeEntryCell()
        {
            this.InitializeComponent();
            this.SetupKeyboardFocusedReverseBinding(nameof(ViewModel.IsFocused));
            ViewModel = new TimeEntryCellViewModel(timeEntryLabel.ViewModel);
        }

        public void Display(Toggl.TogglTimeEntryView item)
        {
            ViewModel.Guid = item.Group ? item.GroupName : item.GUID;
            ViewModel.IsGroup = item.Group;
            ViewModel.IsGroupExpanded = item.GroupOpen;
            ViewModel.GroupName = item.GroupName;
            ViewModel.IsSubItem = !item.Group && item.GroupItemCount == 0;
            ViewModel.DurationInSeconds = item.DurationInSeconds;
            ViewModel.TimeEntryLabel.SetTimeEntry(item);
            timeEntryInfoPanel.SetTimeEntry(item);

            this.entryHoverColor = hoverColor;
            this.EntryBackColor = idleBackColor;

            this.unsyncedIcon.ShowOnlyIf(item.Unsynced);
            this.lockedIcon.ShowOnlyIf(item.Locked);

            this.setupGroupedMode(item);
        }

        private void setupGroupedMode(Toggl.TogglTimeEntryView item)
        {
            String groupItemsText = "";
            String groupIcon = "group_icon_closed.png";
            Color backColor = idleBackColor;
            Color color = defaultForegroundColor;
            int lead = 16;
            Visibility visibility = Visibility.Collapsed;
            SubItem = (item.GroupItemCount == 0 && !item.Group);
            // subitem that is open
            if (SubItem)
            {
                lead = 26;
                backColor = subItemBackColor;
                color = (Color)ColorConverter.ConvertFromString("#FF696969");
            }

            if (item.Group)
            {
                if (item.GroupOpen)
                {
                    // Change group icon to green arrow
                     groupIcon = "group_icon_open.png";
                }
                else
                {
                    // Show count
                    groupItemsText = Convert.ToString(item.GroupItemCount);
                }
                visibility = Visibility.Visible;
            }
            this.EntryBackColor = backColor;
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

        private void entryMouseEnter(object sender, MouseEventArgs e)
        {
            this.EntryBackColor = this.entryHoverColor;
        }

        private void entryMouseLeave(object sender, MouseEventArgs e)
        {
            if (SubItem)
            {
                this.EntryBackColor = subItemBackColor;
            }
            else
            {
                this.EntryBackColor = idleBackColor;
            }
        }
    }
}
