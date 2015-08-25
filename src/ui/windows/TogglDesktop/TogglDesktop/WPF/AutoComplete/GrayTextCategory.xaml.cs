
namespace TogglDesktop.WPF.AutoComplete
{
    public partial class GrayTextCategory : IRecyclable
    {
        public GrayTextCategory()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public GrayTextCategory Initialised(string text)
        {
            this.categoryName.Text = text;
            return this;
        }

        public void Recycle()
        {
            StaticObjectPool.Push(this);
        }
    }
}
