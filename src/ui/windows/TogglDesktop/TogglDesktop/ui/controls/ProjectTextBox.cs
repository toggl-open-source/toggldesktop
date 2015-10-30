using System;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop
{
    class ProjectTextBox : ExtendedTextBox
    {
        private string taskText;
        private TextBlock taskTextBlock;

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            this.taskTextBlock = (TextBlock)this.GetTemplateChild("taskTextBlock");
            this.updateTaskText();
        }

        protected override void OnTextChanged(TextChangedEventArgs e)
        {
            if (!this.IsTextChangingProgrammatically)
            {
                this.setTask(null);
            }

            base.OnTextChanged(e);
        }

        public void SetText(string project, string task)
        {
            this.SetText(project);
            this.setTask(task);
        }

        private void setTask(string task)
        {
            this.taskText = task;
            this.updateTaskText();
        }

        private void updateTaskText()
        {
            if (this.taskTextBlock == null)
                return;

            if (string.IsNullOrEmpty(this.taskText))
            {
                this.taskTextBlock.Text = "";
                this.taskTextBlock.Visibility = Visibility.Collapsed;
            }
            else
            {
                this.taskTextBlock.Text = "- " + this.taskText;
                this.taskTextBlock.Visibility = Visibility.Visible;
            }
        }
    }
}