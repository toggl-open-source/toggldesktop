using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    partial class TimerEntry
    {
        public TimerEntry(Toggl.AutocompleteItem item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.InitializeComponent();
            this.setText(item);
        }

        private void setText(Toggl.AutocompleteItem item)
        {
            if (string.IsNullOrEmpty(item.ProjectLabel))
            {
                this.project.Visibility = Visibility.Collapsed;
            }
            else
            {
                this.project.Text = "• " + item.ProjectLabel;
                this.project.Foreground = getProjectColorBrush(ref item);
                this.project.Visibility = Visibility.Visible;
            }

            setOptionalTextBlockText(this.description, item.Description);
            setOptionalTextBlockText(this.task, item.TaskLabel);
            setOptionalTextBlockText(this.client, item.ClientLabel);
        }

        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            showOnlyIf(textBlock, !string.IsNullOrEmpty(text));
        }

        private static void showOnlyIf(UIElement element, bool condition)
        {
            element.Visibility = condition ? Visibility.Visible : Visibility.Collapsed;
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.AutocompleteItem item)
        {
            var colourString = string.IsNullOrEmpty(item.ProjectColor) ? "#999999" : item.ProjectColor;
            var color = (Color)(ColorConverter.ConvertFromString(colourString) ?? Color.FromRgb(153, 153, 153));
            return new SolidColorBrush(color);
        }
    }
}
