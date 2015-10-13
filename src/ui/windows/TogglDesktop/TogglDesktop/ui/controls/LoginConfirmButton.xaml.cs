using System;
using System.Windows.Controls;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for LoginConfirmButton.xaml
    /// </summary>
    public partial class LoginConfirmButton : UserControl
    {
        public event EventHandler Click;

        public LoginConfirmButton()
        {
            InitializeComponent();

            this.button.Click += (o, e) =>
                {
                    if (this.Click != null)
                        this.Click(this, EventArgs.Empty);
                };
        }

        public string Text
        {
            get { return (string)this.button.Content; }
            set { this.button.Content = value; }
        }
    }
}
