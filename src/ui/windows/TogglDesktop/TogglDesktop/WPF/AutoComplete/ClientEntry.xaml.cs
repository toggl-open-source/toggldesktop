using System;
using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    sealed class ClientEntrySample
    {
        public string ClientName { get { return "Big money company"; } }

        public Color BackgroundColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    partial class ClientEntry
    {
        public ClientEntry(Toggl.Model item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.ClientName = item.Name;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty ClientNameProperty = DependencyProperty
            .Register("ClientName", typeof(string), typeof(ClientEntry));

        public string ClientName
        {
            get { return (string)this.GetValue(ClientNameProperty); }
            set { this.SetValue(ClientNameProperty, value); }
        }
        #endregion
    }
}
