using System;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompleteControls
{
    partial class TimerEntry : IRecyclable
    {
        public TimerEntry()
        {
            this.InitializeComponent();
        }

        public TimerEntry Initialised(Toggl.TogglAutocompleteView item, Action selectWithClick)
        {
            this.setText(item);
            this.setClickAction(selectWithClick);
            return this;
        }

        private void setText(Toggl.TogglAutocompleteView item)
        {
            if (string.IsNullOrEmpty(item.ProjectLabel))
            {
                this.project.Visibility = Visibility.Collapsed;
            }
            else
            {
                this.project.Text = "• " + item.ProjectLabel;
                this.project.Foreground = Utils.ProjectColorBrushFromString(item.ProjectColor);
                this.project.Visibility = Visibility.Visible;
            }

            setOptionalTextBlockText(this.description, item.Description);
            setOptionalTextBlockText(this.task, string.IsNullOrEmpty(item.TaskLabel) ? "" : "- " + item.TaskLabel);
            setOptionalTextBlockText(this.client, string.IsNullOrEmpty(item.ClientLabel) ? "" : "- " + item.ClientLabel);
        }

        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            textBlock.ShowOnlyIf(!string.IsNullOrEmpty(text));
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            StaticObjectPool.Push(this);
        }
    }
}
