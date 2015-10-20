using System;

namespace TogglDesktop.AutoCompleteControls
{
    partial class ProjectEntry : IRecyclable
    {
        public ProjectEntry()
        {
            this.InitializeComponent();
        }

        public ProjectEntry Initialised(Toggl.TogglAutocompleteView item, Action selectWithClick, string overideText = null)
        {
            var colorBrush = Utils.ProjectColorBrushFromString(item.ProjectColor);
            colorBrush.Freeze();
            this.projectColor.Background = colorBrush;
            this.project.Text = overideText ?? item.ProjectLabel;
            this.task.Text = string.IsNullOrEmpty(item.TaskLabel) ? "" : "- " + item.TaskLabel;
            this.setClickAction(selectWithClick);
            return this;
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            StaticObjectPool.Push(this);
        }
    }
}
