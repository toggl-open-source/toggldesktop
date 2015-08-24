using System;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    partial class ProjectEntry : IRecyclable
    {
        public ProjectEntry()
        {
            this.InitializeComponent();
        }

        public ProjectEntry Initialised(Toggl.TogglAutocompleteView item, Action selectWithClick, string overideText = null)
        {
            var colorBrush = new SolidColorBrush(getProjectColor(ref item));
            colorBrush.Freeze();
            this.projectColor.Background = colorBrush;
            this.project.Text = overideText ?? item.ProjectLabel;
            this.task.Text = string.IsNullOrEmpty(item.TaskLabel) ? "" : "- " + item.TaskLabel;
            this.setClickAction(selectWithClick);
            return this;
        }

        private static Color getProjectColor(ref Toggl.TogglAutocompleteView item)
        {
            var projectColourString = string.IsNullOrEmpty(item.ProjectColor) ? "#999999" : item.ProjectColor;
            var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
            return projectColor;
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            StaticObjectPool.Push(this);
        }
    }
}
