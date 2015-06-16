using System.Windows.Controls;

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
    }
}
