
namespace TogglDesktop.WPF.AutoComplete
{
    public partial class GrayTextCategory
    {
        public GrayTextCategory(string text)
        {
            this.DataContext = this;
            this.InitializeComponent();
            this.categoryName.Text = text;
        }
    }
}
