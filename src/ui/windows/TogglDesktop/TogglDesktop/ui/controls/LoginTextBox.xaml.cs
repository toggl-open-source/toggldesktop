namespace TogglDesktop
{
    public partial class LoginTextBox
    {
        public LoginTextBox()
        {
            this.InitializeComponent();
            this.textBox.TextChanged += (o, e) => this.setEmptyLabelVisibility();
        }

        public string Text
        {
            get
            {
                return this.textBox.Text;
            }
            set
            {
                this.textBox.Text = value;
            }
        }

        public string EmptyText
        {
            get
            {
                return (string)this.emptyLabel.Content;
            }
            set
            {
                this.emptyLabel.Content = value;
            }
        }

        private void setEmptyLabelVisibility()
        {
            this.emptyLabel.ShowOnlyIf(this.textBox.Text == "");
        }

        new public void Focus()
        {
            this.textBox.Focus();
        }
    }
}
