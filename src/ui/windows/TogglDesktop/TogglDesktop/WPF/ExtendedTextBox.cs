using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop.WPF
{
    class ExtendedTextBox : TextBox
    {
        public bool IsTextChangingProgrammatically { get; private set; }

        public void SetText(string text)
        {
            this.IsTextChangingProgrammatically = true;
            this.Text = text;
            this.IsTextChangingProgrammatically = false;
        }

        protected override void OnMouseDoubleClick(MouseButtonEventArgs e)
        {
            this.SelectAll();
            e.Handled = true;
        }
    }
}
