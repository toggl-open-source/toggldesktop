using System.Windows;
using System.Windows.Media;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    sealed class ClientAutoCompleteEntrySample
    {
        public string ClientName { get { return "Big money company"; } }

        public Color BackgroundColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    public partial class ClientAutoCompleteEntry : ISelectable
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);

        private bool selected;

        public ClientAutoCompleteEntry(Toggl.Model item)
        {
            this.DataContext = this;
            this.ClientName = item.Name;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty ClientNameProperty = DependencyProperty
            .Register("ClientName", typeof(string), typeof(ClientAutoCompleteEntry));

        public string ClientName
        {
            get { return (string)this.GetValue(ClientNameProperty); }
            set { this.SetValue(ClientNameProperty, value); }
        }
        
        public static readonly DependencyProperty BackgroundColorProperty = DependencyProperty
            .Register("BackgroundColor", typeof(Color), typeof(ClientAutoCompleteEntry));


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
