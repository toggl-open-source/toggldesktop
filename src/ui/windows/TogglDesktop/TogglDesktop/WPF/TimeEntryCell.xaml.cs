using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using TogglDesktop.Diagnostics;

namespace TogglDesktop.WPF
{
    public partial class TimeEntryCell
    {
        private static readonly Color idleBackColor = Color.FromRgb(255, 255, 255);
        private static readonly Color hoverColor = Color.FromRgb(244, 244, 244);
        private static readonly Color hoverColorSelected = Color.FromRgb(255, 255, 255);

        private Color entryHoverColor;

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

        public Color EntryBackColor
        {
            get { return (Color)this.GetValue(EntryBackColorProperty); }
            set { this.SetValue(EntryBackColorProperty, value); }
        }
        public static readonly DependencyProperty EntryBackColorProperty = DependencyProperty
            .Register("EntryBackColor", typeof(Color), typeof(TimeEntryCell), new FrameworkPropertyMetadata(idleBackColor));

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

        public void Display(Toggl.TogglTimeEntryView item)
        {
            this.guid = item.GUID;

            this.labelDescription.Text = item.Description == "" ? "(no description)" : item.Description;
            
            var projectColorBrush = getProjectColorBrush(ref item);

            this.projectColor.Fill = projectColorBrush;
            this.labelProject.Foreground = projectColorBrush;
            this.labelProject.Text = item.ClientLabel == "" ? item.ProjectLabel : "• " + item.ProjectLabel;
            setOptionalTextBlockText(this.labelClient, item.ClientLabel);
            setOptionalTextBlockText(this.labelTask, item.TaskLabel == "" ? "" : item.TaskLabel + " -");
            this.labelDuration.Text = item.Duration;
            showOnlyIf(this.billabeIcon, item.Billable);
            showOnlyIf(this.tagsIcon, !string.IsNullOrEmpty(item.Tags));

            this.projectRow.Height = item.ProjectLabel == "" ? new GridLength(0) : GridLength.Auto;

            showOnlyIf(this.dayHeader, item.IsHeader);

            if (item.IsHeader)
            {
                this.labelFormattedDate.Text = item.DateHeader;
                this.labelDateDuration.Text = item.DateDuration;
            }

            this.updateToolTips(item);
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

        private static void showOnlyIf(FrameworkElement element, bool condition)
        {
            element.Visibility = condition ? Visibility.Visible : Visibility.Collapsed;
        }

        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            showOnlyIf(textBlock, !string.IsNullOrEmpty(text));
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TogglTimeEntryView item)
        {
            var colourString = string.IsNullOrEmpty(item.Color) ? "#999999" : item.Color;
            var color = (Color)(ColorConverter.ConvertFromString(colourString) ?? Color.FromRgb(153, 153, 153));
            return new SolidColorBrush(color);
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
