using System;
using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    sealed class ProjectEntrySample
    {
        public Color ProjectColor { get { return Color.FromRgb(153, 153, 153); } }

        public string ProjectName { get { return "Hello. Yes, this is project"; } }

        public Color BackgroundColor { get { return Color.FromRgb(244, 244, 244); } }
    }

    partial class ProjectEntry
    {
        public ProjectEntry(Toggl.AutocompleteItem item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.ProjectColor = getProjectColor(ref item);
            this.ProjectName = item.ProjectLabel;
            this.InitializeComponent();
        }

        private static Color getProjectColor(ref Toggl.AutocompleteItem item)
        {
            var projectColourString = string.IsNullOrEmpty(item.Project) ? "#999999" : item.Project;
            var projectColor = (Color)ColorConverter.ConvertFromString(projectColourString);
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
        
        #endregion

    }
}
