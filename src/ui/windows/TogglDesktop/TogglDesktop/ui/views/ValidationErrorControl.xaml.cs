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

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for ValidationErrorControl.xaml
    /// </summary>
    public partial class ValidationErrorControl : UserControl
    {
        public ValidationErrorControl()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty ErrorTextProperty =
            DependencyProperty.Register(
                "ErrorText", typeof(string),
                typeof(ValidationErrorControl)
            );
        public string ErrorText
        {
            get => (string)GetValue(ErrorTextProperty);
            set => SetValue(ErrorTextProperty, value);
        }
    }
}
