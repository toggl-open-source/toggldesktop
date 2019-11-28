using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop
{
    class ExtendedTextBox : TextBox
    {
        public bool SelectAllOnKeyboardFocus { get; set; }

        public ExtendedTextBox()
        {
            this.SelectAllOnKeyboardFocus = true;
        }

        public event TextChangedEventHandler TextChangedByUser;

        public new string Text
        {
            get => base.Text;
            set
            {
                this.IsTextChangingProgrammatically = true;
                base.Text = value;
                this.IsTextChangingProgrammatically = false;
            }
        }

        protected bool IsTextChangingProgrammatically { get; private set; }

        protected override void OnTextChanged(TextChangedEventArgs e)
        {
            base.OnTextChanged(e);
            if (!IsTextChangingProgrammatically)
            {
                TextChangedByUser?.Invoke(this, e);
            }
        }

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            if (!this.IsFocused)
            {
                this.SelectionLength = 0;
            }

            if (e.ClickCount == 3)
            {
                this.SelectAll();
            }

            base.OnMouseDown(e);
        }

        protected override void OnGotKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            if (!(Mouse.LeftButton == MouseButtonState.Pressed && this.IsMouseOver))
            {
                this.onFocusedByTab();
            }

            base.OnGotKeyboardFocus(e);
        }

        private void onFocusedByTab()
        {
            if (this.SelectAllOnKeyboardFocus)
            {
                this.SelectAll();
            }

            if (this.SelectedText == "")
            {
                this.CaretIndex = this.Text.Length;
            }
        }

        protected override void OnLostKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            if (this.SelectAllOnKeyboardFocus)
            {
                this.SelectionLength = 0;
            }

            base.OnLostKeyboardFocus(e);
        }
    }
}
