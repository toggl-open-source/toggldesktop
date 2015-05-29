using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for LoginTextBox.xaml
    /// </summary>
    public partial class LoginPasswordBox : UserControl
    {
        public string Text
        {
            get
            {
                return passwordBox.Password;
            }
            set
            {
                passwordBox.Password = value;
            }
        }

        public string EmptyText
        {
            get
            {
                return (string)emptyLabel.Content;
            }
            set
            {
                emptyLabel.Content = value;
            }
        }

        public LoginPasswordBox()
        {
            InitializeComponent();

            passwordBox.PasswordChanged += (o, e) => setEmptyLabelVisibility();
        }

        private void setEmptyLabelVisibility()
        {
            emptyLabel.Visibility = passwordBox.Password == "" ? Visibility.Visible : Visibility.Hidden;
        }

        internal void Clear()
        {
            this.passwordBox.Clear();
        }

        new public void Focus()
        {
            passwordBox.Focus();
        }

    }
}
