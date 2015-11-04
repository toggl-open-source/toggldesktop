
namespace TogglDesktop.AutoCompleteControls
{
    public partial class WorkspaceCategory : IRecyclable
    {
        public WorkspaceCategory()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public WorkspaceCategory Initialised(string text)
        {
            this.workspaceName.Text = text;
            return this;
        }

        public void Recycle()
        {
            StaticObjectPool.Push(this);
        }
    }
}
