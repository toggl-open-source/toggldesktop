using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop
{
    public partial class PasswordRuleView : UserControl
    {
        public PasswordRuleView()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty IsRuleSatisfiedProperty = DependencyProperty.Register(
            "IsRuleSatisfied", typeof(bool), typeof(PasswordRuleView), new PropertyMetadata(default(bool)));

        public bool IsRuleSatisfied
        {
            get { return (bool)GetValue(IsRuleSatisfiedProperty); }
            set { SetValue(IsRuleSatisfiedProperty, value); }
        }

        public static readonly DependencyProperty IsValidationFailedProperty = DependencyProperty.Register(
            "IsValidationFailed", typeof(bool), typeof(PasswordRuleView), new PropertyMetadata(default(bool)));

        public bool IsValidationFailed
        {
            get { return (bool)GetValue(IsValidationFailedProperty); }
            set { SetValue(IsValidationFailedProperty, value); }
        }

        public static readonly DependencyProperty TextProperty = DependencyProperty.Register(
            "Text", typeof(string), typeof(PasswordRuleView), new PropertyMetadata(default(string)));

        public string Text
        {
            get { return (string)GetValue(TextProperty); }
            set { SetValue(TextProperty, value); }
        }
    }
}