
using System.Windows.Controls;

namespace TogglDesktop.AutoCompleteControls
{
    public partial class ClientCategory : IRecyclable
    {
        public ClientCategory()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public ClientCategory Initialised(string text, out Panel childrenPanel)
        {
            this.clientName.Text = text;
            childrenPanel = this.projectPanel;
            return this;
        }

        public void Recycle()
        {
            this.projectPanel.Children.Clear();
            StaticObjectPool.Push(this);
        }
    }
}
