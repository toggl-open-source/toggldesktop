using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    sealed class TimeEntryCellSample
    {
        public Color EntryBackColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    public partial class TimeEntryCell
    {
        private static readonly Color idleBackColor = Color.FromRgb(255, 255, 255);
        private static readonly Color hoverColor = Color.FromRgb(244, 244, 244);
        private static readonly Color hoverColorSelected = Color.FromRgb(255, 255, 255);
        private static readonly Color subItemBackColor = Color.FromRgb(240, 240, 240);
        private static readonly Color defaultForegroundColor = Color.FromRgb(0, 0, 0);

        private Color entryHoverColor = hoverColor;

        private string guid { get; set; }
        private string groupName { get; set; }
        private Boolean group = false;

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                if (value == this.selected)
                    return;
                var color = value ? hoverColorSelected : hoverColor;
                this.entryHoverColor = color;
                if (this.IsMouseOver)
                {
                    this.EntryBackColor = color;
                }
                this.selected = value;
            }
        }

        public bool IsFloating
        {
            set
            {
                if (value == false)
                    return;

                this.entrySeperator.Visibility = Visibility.Collapsed;
                this.entryGrid.Height = 59;
            }
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

        public Color GroupIconPath { get; set; }

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

        public TimeEntryCellDayHeader DayHeader { get; private set; }
        public bool confirmlessDelete = false;

        private readonly ToolTip descriptionToolTip = new ToolTip();
        private readonly ToolTip taskProjectClientToolTip = new ToolTip();
        private readonly ToolTip durationToolTip = new ToolTip();
        private readonly ToolTip tagsToolTip = new ToolTip();
        private bool selected;
        private Point mouseDownPosition;
        private bool isMouseDown;
        private bool dragging;

        public TimeEntryCell()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public void Imitate(TimeEntryCell cell)
        {
            this.guid = cell.guid;
            this.labelDescription.Text = cell.labelDescription.Text;
            this.projectColor.Fill = cell.projectColor.Fill;

            this.labelProject.Foreground = cell.labelProject.Foreground;
            this.labelProject.Text = cell.labelProject.Text;
            setOptionalTextBlockText(this.labelClient, cell.labelClient.Text);
            setOptionalTextBlockText(this.labelTask, cell.labelTask.Text);
            this.labelDuration.Text = cell.labelDuration.Text;
            this.billabeIcon.Visibility = cell.billabeIcon.Visibility;
            this.tagsIcon.Visibility = cell.tagsIcon.Visibility;
            this.tagsCount.Text = cell.tagsCount.Text;

            this.projectRow.Height = cell.projectRow.Height;

            this.entryHoverColor = cell.entryHoverColor;
            this.EntryBackColor = cell.EntryBackColor;

            this.unsyncedIcon.Visibility = cell.unsyncedIcon.Visibility;
            this.lockedIcon.Visibility = cell.lockedIcon.Visibility;
            this.groupItemsBack.Visibility = cell.groupItemsBack.Visibility;
            this.groupImage.Source = cell.groupImage.Source;
            this.groupItems.Text = cell.groupItems.Text;

            this.imitateTooltips(cell);
        }

        public void Display(Toggl.TogglTimeEntryView item, TimeEntryCellDayHeader dayHeader)
        {
            this.guid = item.GUID;
            this.DayHeader = dayHeader;

            this.labelDescription.Text = item.Description == "" ? "(no description)" : item.Description;

            var projectColorBrush = Utils.ProjectColorBrushFromString(item.Color);

            this.projectColor.Fill = projectColorBrush;
            this.labelProject.Foreground = projectColorBrush;
            this.labelProject.Text = item.ClientLabel.IsNullOrEmpty() ? item.ProjectLabel : "• " + item.ProjectLabel;
            setOptionalTextBlockText(this.labelClient, item.ClientLabel);
            setOptionalTextBlockText(this.labelTask, item.TaskLabel.IsNullOrEmpty() ? "" : item.TaskLabel + " -");
            this.labelDuration.Text = item.Duration;
            this.billabeIcon.ShowOnlyIf(item.Billable);

            this.confirmlessDelete = (item.Description.Length == 0
                    && item.DurationInSeconds < 15 && item.PID == 0);

            if (string.IsNullOrEmpty(item.Tags))
            {
                this.tagsIcon.Visibility = Visibility.Collapsed;
            }
            else
            {
                this.tagsIcon.Visibility = Visibility.Visible;
                this.tagsCount.Text = item.Tags.CountSubstrings(Toggl.TagSeparator).ToString();
            }

            this.projectRow.Height = item.ProjectLabel == "" ? new GridLength(0) : GridLength.Auto;

            this.entrySeperator.ShowOnlyIf(!item.IsHeader);

            this.entryHoverColor = hoverColor;
            this.EntryBackColor = idleBackColor;

            this.unsyncedIcon.ShowOnlyIf(item.Unsynced);
            this.lockedIcon.ShowOnlyIf(item.Locked);

            this.updateToolTips(item);

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
            group = item.Group;
            groupName = item.GroupName;
            SubItem = (item.GroupItemCount > 0 && item.GroupOpen && !item.Group);
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
            labelDescription.Foreground = new System.Windows.Media.SolidColorBrush(color);
            labelDuration.Foreground = new System.Windows.Media.SolidColorBrush(color);
            this.EntryBackColor = backColor;
            this.groupItemsBack.Visibility = visibility;
            groupItems.Text = groupItemsText;
            this.groupImage.Source = new BitmapImage(new Uri("pack://application:,,,/TogglDesktop;component/Resources/" + groupIcon));
            // leading margin
            descriptionGrid.Margin = new Thickness(lead, 0, 0, 0);
        }

        private void imitateTooltips(TimeEntryCell cell)
        {
            setToolTipIfNotEmpty(this.labelDescription, this.descriptionToolTip, cell.descriptionToolTip.Content as string);
            setToolTipIfNotEmpty(this.labelTask, this.taskProjectClientToolTip, cell.taskProjectClientToolTip.Content as string);
            setToolTipIfNotEmpty(this.labelProject, this.taskProjectClientToolTip, cell.taskProjectClientToolTip.Content as string);
            setToolTipIfNotEmpty(this.labelClient, this.taskProjectClientToolTip, cell.taskProjectClientToolTip.Content as string);

            setToolTipIfNotEmpty(this.labelDuration, this.durationToolTip, cell.durationToolTip.Content as string);

            if (this.tagsIcon.Visibility == Visibility.Visible)
            {
                this.tagsToolTip.Content = cell.tagsToolTip.Content;
                this.tagsIcon.ToolTip = this.tagsToolTip;
            }
        }

        private void updateToolTips(Toggl.TogglTimeEntryView item)
        {
            setToolTipIfNotEmpty(this.labelDescription, this.descriptionToolTip, item.Description);
            setToolTipIfNotEmpty(this.labelTask, this.taskProjectClientToolTip, item.ProjectAndTaskLabel);
            setToolTipIfNotEmpty(this.labelProject, this.taskProjectClientToolTip, item.ProjectAndTaskLabel);
            setToolTipIfNotEmpty(this.labelClient, this.taskProjectClientToolTip, item.ProjectAndTaskLabel);

            if (item.DurOnly)
            {
                this.labelDuration.ToolTip = null;
            }
            else
            {
                this.labelDuration.ToolTip = this.durationToolTip;
                this.durationToolTip.Content = item.StartTimeString + " - " + item.EndTimeString;
            }

            if (this.tagsIcon.Visibility == Visibility.Visible)
            {
                this.tagsToolTip.Content = item.Tags.Replace(Toggl.TagSeparator, ", ");
                this.tagsIcon.ToolTip = this.tagsToolTip;
            }
        }

        private static void setToolTipIfNotEmpty(FrameworkElement element, ToolTip tooltip, string content)
        {
            if (string.IsNullOrEmpty(content))
            {
                element.ToolTip = null;
            }
            else
            {
                tooltip.Content = content;
                element.ToolTip = tooltip;
            }
        }

        #region display helpers

        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            textBlock.ShowOnlyIf(!string.IsNullOrEmpty(text));
        }

        #endregion

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
            if (group)
            {
                Toggl.ToggleEntriesGroup(groupName);
                e.Handled = true;
                return;
            }
            using (Performance.Measure("opening edit view from cell, focussing " + focusedField))
            {
                Toggl.Edit(this.guid, false, focusedField);
            }
            e.Handled = true;
        }

        #endregion

        #region drag drop

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            this.mouseDownPosition = e.GetPosition(null);
            this.isMouseDown = true;
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            this.tryStartDrag(e);
        }

        public void MoveToDay(DateTime date)
        {
            Toggl.SetTimeEntryDate(this.guid, date);
        }

        public void DeleteTimeEntry()
        {
            if (this.confirmlessDelete)
            {
                Toggl.DeleteTimeEntry(this.guid);
                return;
            }
            Toggl.AskToDeleteEntry(this.guid);
        }

        private void tryStartDrag(MouseEventArgs e, bool ignoreDistance = false)
        {
            if (!this.group && this.IsEnabled && this.isMouseDown && e.LeftButton == MouseButtonState.Pressed)
            {
                var d = e.GetPosition(null) - this.mouseDownPosition;

                if (!ignoreDistance)
                {
                    if (Math.Abs(d.X) < SystemParameters.MinimumHorizontalDragDistance ||
                        Math.Abs(d.Y) < SystemParameters.MinimumVerticalDragDistance)
                        return;
                }

                this.EntryBackColor = this.entryHoverColor;
                this.dragging = true;
                TimeEntryCellDragImposter.Start(this);
                DragDrop.DoDragDrop(this, new DataObject("time-entry-cell", this), DragDropEffects.Move);
                TimeEntryCellDragImposter.Stop();
                this.dragging = false;
                this.EntryBackColor = idleBackColor;

                e.Handled = true;
            }

            this.isMouseDown = false;
        }

        protected override void OnGiveFeedback(GiveFeedbackEventArgs e)
        {
            TimeEntryCellDragImposter.Update();
        }

        #endregion

        private void buttonContinue_Click(object sender, RoutedEventArgs e)
        {
            using (Performance.Measure("continuing time entry from cell"))
            {
                Toggl.Continue(this.guid);
            }
        }

        private void entryMouseEnter(object sender, MouseEventArgs e)
        {
            this.EntryBackColor = this.entryHoverColor;
        }

        private void entryMouseLeave(object sender, MouseEventArgs e)
        {
            if (this.dragging)
                return;

            this.tryStartDrag(e, true);
            if (SubItem)
            {
                this.EntryBackColor = subItemBackColor;
            }
            else
            {
                this.EntryBackColor = idleBackColor;
            }
            this.isMouseDown = false;
        }

    }
}
