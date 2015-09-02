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
    public partial class LoginTextBox : UserControl
    {
        public LoginTextBox()
        {
            InitializeComponent();
            textBox.TextChanged += (o, e) => setEmptyLabelVisibility();
        }

        public string Text
        {
            get
            {
                return textBox.Text;
            }
            set
            {
                textBox.Text = value;
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

        private void setEmptyLabelVisibility()
        {
            emptyLabel.Visibility = textBox.Text == "" ? Visibility.Visible : Visibility.Hidden;
        }

        new public void Focus()
        {
            textBox.Focus();
        }
    }
}
