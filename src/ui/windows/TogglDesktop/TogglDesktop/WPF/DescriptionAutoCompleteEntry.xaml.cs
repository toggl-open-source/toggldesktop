using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF
{
    sealed class DescriptionAutoCompleteEntrySample
    {
        public Color ProjectColor { get { return Color.FromRgb(153, 153, 153); } }

        public string Description { get { return "Description? Hello. Yes, this is test"; } }

        public Color BackgroundColor { get { return Color.FromRgb(244, 244, 244); } }
    }

    public partial class DescriptionAutoCompleteEntry
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);

        private bool selected;

        public DescriptionAutoCompleteEntry(Toggl.AutocompleteItem item)
        {
            this.DataContext = this;
            this.ProjectColor = getProjectColor(ref item);
            this.Description = item.Description;
            InitializeComponent();
        }

        private static Color getProjectColor(ref Toggl.AutocompleteItem item)
        {
            var projectColourString = "#999999";
            var projectColor = (Color)ColorConverter.ConvertFromString(projectColourString);
            return projectColor;
        }

        #region dependency properties

        public static readonly DependencyProperty ProjectColorProperty = DependencyProperty
            .Register("ProjectColor", typeof(Color), typeof(DescriptionAutoCompleteEntry));

        public Color ProjectColor
        {
            get { return (Color)this.GetValue(ProjectColorProperty); }
            set { this.SetValue(ProjectColorProperty, value); }
        }

        public static readonly DependencyProperty DescriptionProperty = DependencyProperty
            .Register("Description", typeof(string), typeof(DescriptionAutoCompleteEntry));

        public string Description
        {
            get { return (string)this.GetValue(DescriptionProperty); }
            set { this.SetValue(DescriptionProperty, value); }
        }
        
        public static readonly DependencyProperty BackgroundColorProperty = DependencyProperty
            .Register("BackgroundColor", typeof(Color), typeof(DescriptionAutoCompleteEntry));


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
