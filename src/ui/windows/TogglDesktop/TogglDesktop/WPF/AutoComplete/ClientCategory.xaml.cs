
namespace TogglDesktop.WPF.AutoComplete
{
    public partial class ClientCategory
    {
        public ClientCategory(string text)
        {
            this.DataContext = this;
            this.InitializeComponent();
            this.clientName.Text = text;
        }
    }
}
