using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for TimeEntryCell.xaml
    /// </summary>
    public partial class TimeEntryCell
    {
        private static readonly Color hoverColor = Color.FromRgb(244, 244, 244);
        private static readonly Color hoverColorSelected = Color.FromRgb(255, 255, 255);

        public string GUID { get; set; }

        public Color EntryHoverColor
        {
            get { return (Color)this.GetValue(EntryHoverColorProperty); }
            set { this.SetValue(EntryHoverColorProperty, value); }
        }
        public static readonly DependencyProperty EntryHoverColorProperty = DependencyProperty
            .Register("EntryHoverColor", typeof(Color), typeof(TimeEntryCell), new FrameworkPropertyMetadata(hoverColor));

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                if (value == this.selected)
                    return;
                this.EntryHoverColor = value ? hoverColorSelected : hoverColor;
                this.selected = value;
            }
        }

        public Color EntryBackColor
        {
            get { return (Color)this.GetValue(EntryBackColorProperty); }
            set { this.SetValue(EntryBackColorProperty, value); }
        }
        public static readonly DependencyProperty EntryBackColorProperty = DependencyProperty
            .Register("EntryBackColor", typeof(Color), typeof(TimeEntryCell), new FrameworkPropertyMetadata(Color.FromArgb(255, 0, 0, 0)));

        private readonly ToolTip descriptionToolTip = new ToolTip();
        private readonly ToolTip taskProjectClientToolTip = new ToolTip();
        private readonly ToolTip durationToolTip = new ToolTip();
        private readonly ToolTip tagsToolTip = new ToolTip();
        private bool selected;

        public TimeEntryCell()
        {
            InitializeComponent();
            DataContext = this;
        }

        public void Display(Toggl.TimeEntry item)
        {
            GUID = item.GUID;

            labelDescription.Text = item.Description == "" ? "(no description)" : item.Description;
            
            var projectColorBrush = getProjectColorBrush(ref item);

            projectColor.Fill = projectColorBrush;
            labelProject.Foreground = projectColorBrush;
            labelProject.Text = (item.ClientLabel.Length > 0) ? "• " + item.ProjectLabel : item.ProjectLabel;
            setOptionalTextBlockText(labelClient, item.ClientLabel);
            setOptionalTextBlockText(labelTask, item.TaskLabel);
            labelDuration.Text = item.Duration;
            showOnlyIf(billabeIcon, item.Billable);
            showOnlyIf(tagsIcon, !string.IsNullOrEmpty(item.Tags));

            showOnlyIf(dayHeader, item.IsHeader);

            if (item.IsHeader)
            {
                labelFormattedDate.Text = item.DateHeader;
                labelDateDuration.Text = item.DateDuration;
            }

            updateToolTips(item);
        }

        private void updateToolTips(Toggl.TimeEntry item)
        {
            setToolTipIfNotEmpty(labelDescription, descriptionToolTip, item.Description);
            setToolTipIfNotEmpty(labelTask, taskProjectClientToolTip, item.ProjectAndTaskLabel);
            setToolTipIfNotEmpty(labelProject, taskProjectClientToolTip, item.ProjectAndTaskLabel);
            setToolTipIfNotEmpty(labelClient, taskProjectClientToolTip, item.ProjectAndTaskLabel);

            if (item.DurOnly)
            {
                this.labelDuration.ToolTip = null;
            }
            else
            {
                this.labelDuration.ToolTip = this.durationToolTip;
                durationToolTip.Content = item.StartTimeString + " - " + item.EndTimeString;
            }

            if (tagsIcon.Visibility == Visibility.Visible)
            {
                tagsToolTip.Content = item.Tags.Replace(Toggl.TagSeparator, ", ");
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

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TimeEntry item)
        {
            var colourString = string.IsNullOrEmpty(item.Color) ? "#999999" : item.Color;
            var color = (Color)(ColorConverter.ConvertFromString(colourString) ?? Color.FromRgb(153, 153, 153));
            return new SolidColorBrush(color);
        }

        #endregion

        #region open edit window event handlers

        private void labelDuration_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Duration);
            e.Handled = true;
        }

        private void labelDescription_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Description);
            e.Handled = true;
        }

        private void labelProject_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
            e.Handled = true;
        }

        private void entry_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            Toggl.Edit(GUID, false, "");
            e.Handled = true;
        }

        #endregion

        private void buttonContinue_Click(object sender, RoutedEventArgs e)
        {
            Toggl.Continue(GUID);
        }


    }
}
