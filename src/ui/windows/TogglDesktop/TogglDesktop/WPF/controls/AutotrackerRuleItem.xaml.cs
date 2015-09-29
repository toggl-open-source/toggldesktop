using System.Windows;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.WPF
{
    public partial class AutotrackerRuleItem : IRecyclable
    {
        private long id;

        public AutotrackerRuleItem()
        {
            this.InitializeComponent();
        }

        public static AutotrackerRuleItem Make(long id, string term, string project)
        {
            var item = StaticObjectPool.PopOrNew<AutotrackerRuleItem>();

            item.id = id;
            item.termText.Text = term;
            item.projectText.Text = project;

            return item;
        }

        public void Recycle()
        {
            this.id = 0;
            StaticObjectPool.Push(this);
        }

        private void onDeleteButtonClick(object sender, RoutedEventArgs e)
        {
            Toggl.DeleteAutotrackerRule(this.id);
        }
    }
}
