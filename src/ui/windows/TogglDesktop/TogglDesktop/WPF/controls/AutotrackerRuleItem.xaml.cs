using System.Windows;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.WPF
{
    public partial class AutotrackerRuleItem : IRecyclable
    {
        public AutotrackerRuleItem()
        {
            this.InitializeComponent();
        }

        public static AutotrackerRuleItem Make(string term, string project)
        {
            var item = StaticObjectPool.PopOrNew<AutotrackerRuleItem>();

            item.termText.Text = term;
            item.projectText.Text = project;

            return item;
        }

        public void Recycle()
        {
            StaticObjectPool.Push(this);
        }

        private void onDeleteButtonClick(object sender, RoutedEventArgs e)
        {

        }
    }
}
