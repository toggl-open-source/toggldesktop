using System.Windows;
using System.Windows.Media;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    sealed class ProjectAutoCompleteEntrySample
    {
        public Color ProjectColor { get { return Color.FromRgb(153, 153, 153); } }

        public string ProjectName { get { return "Hello. Yes, this is project"; } }

        public Color BackgroundColor { get { return Color.FromRgb(244, 244, 244); } }
    }

    public partial class ProjectAutoCompleteEntry : ISelectable
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);

        private bool selected;

        public ProjectAutoCompleteEntry(Toggl.AutocompleteItem item)
        {
            this.DataContext = this;
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
            .Register("ProjectColor", typeof(Color), typeof(ProjectAutoCompleteEntry));

        public Color ProjectColor
        {
            get { return (Color)this.GetValue(ProjectColorProperty); }
            set { this.SetValue(ProjectColorProperty, value); }
        }

        public static readonly DependencyProperty ProjectNameProperty = DependencyProperty
            .Register("ProjectName", typeof(string), typeof(ProjectAutoCompleteEntry));

        public string ProjectName
        {
            get { return (string)this.GetValue(ProjectNameProperty); }
            set { this.SetValue(ProjectNameProperty, value); }
        }
        
        public static readonly DependencyProperty BackgroundColorProperty = DependencyProperty
            .Register("BackgroundColor", typeof(Color), typeof(ProjectAutoCompleteEntry));


        public Color BackgroundColor
        {
            get { return (Color)this.GetValue(BackgroundColorProperty); }
            set { this.SetValue(BackgroundColorProperty, value); }
        }

        #endregion

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                if (this.selected == value)
                    return;
                this.BackgroundColor = value ? backgroundColorSelected : backgroundColor;
                this.selected = value;
            }
        }

    }
}
