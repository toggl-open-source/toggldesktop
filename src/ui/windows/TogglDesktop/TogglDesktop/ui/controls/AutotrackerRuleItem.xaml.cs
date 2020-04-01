using System.Windows;

namespace TogglDesktop
{
    public partial class AutotrackerRuleItem : IRecyclable
    {
        private long id;

        public AutotrackerRuleItem()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public static readonly DependencyProperty IsSelectedProperty = DependencyProperty.Register(
            "IsSelected", typeof(bool), typeof(AutotrackerRuleItem), new PropertyMetadata(default(bool)));

        public bool IsSelected
        {
            get { return (bool) GetValue(IsSelectedProperty); }
            set { SetValue(IsSelectedProperty, value); }
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
            this.IsSelected = false;
            StaticObjectPool.Push(this);
        }

        private void onDeleteButtonClick(object sender, RoutedEventArgs e)
        {
            this.DeleteRule();
        }

        public void DeleteRule()
        {
            Toggl.DeleteAutotrackerRule(this.id);
        }
    }
}
