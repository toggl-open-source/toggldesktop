using System;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    partial class ProjectEntry
    {
        public ProjectEntry(Toggl.TogglAutocompleteView item, Action selectWithClick, string overideText = null)
            : base(selectWithClick)
        {
            this.InitializeComponent();
            var colorBrush = new SolidColorBrush(getProjectColor(ref item));
            colorBrush.Freeze();
            this.projectColor.Background = colorBrush;
            this.project.Text = overideText ?? item.ProjectLabel;
            this.task.Text = string.IsNullOrEmpty(item.TaskLabel) ? "" : "- " + item.TaskLabel;
        }

        private static Color getProjectColor(ref Toggl.TogglAutocompleteView item)
        {
            var projectColourString = string.IsNullOrEmpty(item.ProjectColor) ? "#999999" : item.ProjectColor;
            var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
            return projectColor;
        }
    }
}
