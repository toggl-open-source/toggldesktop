using System.Windows;
using System.Windows.Media;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    sealed class StringAutoCompleteEntrySample
    {
        public string Text { get { return "This is text"; } }

        public Color BackgroundColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    public partial class StringAutoCompleteEntry : ISelectable
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);

        private bool selected;

        public StringAutoCompleteEntry(string item)
        {
            this.DataContext = this;
            this.Description = item;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty TextProperty = DependencyProperty
            .Register("Text", typeof(string), typeof(StringAutoCompleteEntry));

        public string Description
        {
            get { return (string)this.GetValue(TextProperty); }
            set { this.SetValue(TextProperty, value); }
        }

        public static readonly DependencyProperty BackgroundColorProperty = DependencyProperty
            .Register("BackgroundColor", typeof(Color), typeof(StringAutoCompleteEntry));


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
