using System;
using System.Windows;

namespace TogglDesktop
{
    public partial class Tag
    {
        public event EventHandler RemoveClicked;

        public Tag()
        {
            this.DataContext = this;
            InitializeComponent();
        }

        # region cached creation

        public static Tag Make(string text)
        {
            return StaticObjectPool.PopOrNew<Tag>().withText(text);
        }

        private Tag withText(string text)
        {
            this.textBlock.Text = text;
            return this;
        }

        public void Dispose()
        {
            this.RemoveClicked = null;
            StaticObjectPool.Push(this);
        }

        #endregion

        private void remove_OnClick(object sender, RoutedEventArgs e)
        {
            if (this.RemoveClicked != null)
                this.RemoveClicked(this, EventArgs.Empty);
        }
    }
}
