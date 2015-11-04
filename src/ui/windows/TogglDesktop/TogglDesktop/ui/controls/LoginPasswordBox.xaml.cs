namespace TogglDesktop
{
    public partial class LoginPasswordBox
    {
        public string Text
        {
            get
            {
                return this.passwordBox.Password;
            }
            set
            {
                this.passwordBox.Password = value;
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

        public LoginPasswordBox()
        {
            this.InitializeComponent();

            this.passwordBox.PasswordChanged += (o, e) => this.setEmptyLabelVisibility();
        }

        private void setEmptyLabelVisibility()
        {
            this.emptyLabel.ShowOnlyIf(this.passwordBox.Password == "");
        }

        internal void Clear()
        {
            this.passwordBox.Clear();
        }

        new public void Focus()
        {
            this.passwordBox.Focus();
        }

    }
}
