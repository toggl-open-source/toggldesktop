using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop
{
    public partial class AutotrackerRuleItem : IRecyclable
    {
        private static readonly Color idleBackColor = Color.FromRgb(255, 255, 255);
        private static readonly Color selectedBackColor = Color.FromRgb(244, 244, 244);

        private long id;

        public AutotrackerRuleItem()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public Color BackColor
        {
            get { return (Color)this.GetValue(BackColorProperty); }
            set { this.SetValue(BackColorProperty, value); }
        }
        public static readonly DependencyProperty BackColorProperty = DependencyProperty
            .Register("BackColor", typeof(Color), typeof(AutotrackerRuleItem), new FrameworkPropertyMetadata(idleBackColor));

        private bool selected;

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                if (this.selected == value)
                    return;
                this.selected = value;
                this.updateBackground();
            }
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
            this.Selected = false;
            this.BackColor = idleBackColor;
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

        protected override void OnMouseEnter(MouseEventArgs e)
        {
            this.updateBackground();
            base.OnMouseEnter(e);
        }
        protected override void OnMouseLeave(MouseEventArgs e)
        {
            this.updateBackground();
            base.OnMouseLeave(e);
        }

        private void updateBackground()
        {
            this.BackColor = (this.selected || this.IsMouseOver)
                ? selectedBackColor : idleBackColor;
        }

    }
}
