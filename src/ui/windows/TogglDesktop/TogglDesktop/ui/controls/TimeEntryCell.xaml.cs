using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
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

        private Color entryHoverColor = hoverColor;

        private string guid { get; set; }

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

        public Color EntryBackColor
        {
            get { return (Color)this.GetValue(EntryBackColorProperty); }
            set { this.SetValue(EntryBackColorProperty, value); }
        }
        public static readonly DependencyProperty EntryBackColorProperty = DependencyProperty
            .Register("EntryBackColor", typeof(Color), typeof(TimeEntryCell), new FrameworkPropertyMetadata(idleBackColor));

        public TimeEntryCellDayHeader DayHeader { get; private set; }

        private readonly ToolTip descriptionToolTip = new ToolTip();
        private readonly ToolTip taskProjectClientToolTip = new ToolTip();
        private readonly ToolTip durationToolTip = new ToolTip();
        private readonly ToolTip tagsToolTip = new ToolTip();
        private bool selected;

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

            this.updateToolTips(item);
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
            using (Performance.Measure("opening edit view from cell, focussing " + focusedField))
            {
                Toggl.Edit(this.guid, false, focusedField);
            }
            e.Handled = true;
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
            this.EntryBackColor = idleBackColor;
        }

    }
}
