using System;
using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    sealed class ProjectEntrySample
    {
        public Color ProjectColor { get { return Color.FromRgb(153, 153, 153); } }

        public string ProjectName { get { return "Hello. Yes, this is project"; } }
        public string TaskName { get { return "Some Task"; } }

        public Color BackgroundColor { get { return Color.FromRgb(244, 244, 244); } }
    }

    partial class ProjectEntry
    {
        public ProjectEntry(Toggl.AutocompleteItem item, Action selectWithClick, string overideText = null)
            : base(selectWithClick)
        {
            this.ProjectColor = getProjectColor(ref item);
            this.ProjectName = overideText ?? item.ProjectLabel;
            this.TaskName = string.IsNullOrEmpty(item.TaskLabel) ? "" : "- " + item.TaskLabel;
            this.InitializeComponent();
        }

        private static Color getProjectColor(ref Toggl.AutocompleteItem item)
        {
            var projectColourString = string.IsNullOrEmpty(item.ProjectColor) ? "#999999" : item.ProjectColor;
            var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
            return projectColor;
        }

        #region dependency properties

        public static readonly DependencyProperty ProjectColorProperty = DependencyProperty
            .Register("ProjectColor", typeof(Color), typeof(ProjectEntry));

        public Color ProjectColor
        {
            get { return (Color)this.GetValue(ProjectColorProperty); }
            set { this.SetValue(ProjectColorProperty, value); }
        }

        public static readonly DependencyProperty ProjectNameProperty = DependencyProperty
            .Register("ProjectName", typeof(string), typeof(ProjectEntry));

        public string ProjectName
        {
            get { return (string)this.GetValue(ProjectNameProperty); }
            set { this.SetValue(ProjectNameProperty, value); }
        }

        public static readonly DependencyProperty TaskNameProperty = DependencyProperty
            .Register("TaskName", typeof(string), typeof(ProjectEntry));

        public string TaskName
        {
            get { return (string)this.GetValue(TaskNameProperty); }
            set { this.SetValue(TaskNameProperty, value); }
        }
        
        #endregion

    }
}
