
namespace TogglDesktop.WPF.AutoComplete
{
    public partial class WorkspaceCategory
    {
        public WorkspaceCategory(string text)
        {
            this.DataContext = this;
            this.InitializeComponent();
            this.workspaceName.Text = text;
        }
    }
}
