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
    /// Interaction logic for LoginConfirmButton.xaml
    /// </summary>
    public partial class LoginConfirmButton : UserControl
    {
        public event EventHandler Click;

        public LoginConfirmButton()
        {
            InitializeComponent();

            button.Click += (o, e) =>
                {
                    if (Click != null)
                        Click(this, EventArgs.Empty);
                };
        }

        public string Text
        {
            get { return (string)button.Content; }
            set { button.Content = value; }
        }
    }
}
