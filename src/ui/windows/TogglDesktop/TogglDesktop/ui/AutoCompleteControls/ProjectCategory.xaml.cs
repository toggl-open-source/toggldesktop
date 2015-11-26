using System;
using System.Windows;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.AutoCompleteControls
{
    public partial class ProjectCategory : IRecyclable, ICollapsable
    {
        public event EventHandler CollapsedChanged;

        private bool collapsed;

        public ProjectCategory()
        {
            this.InitializeComponent();
        }

        public bool Collapsed
        {
            get { return this.collapsed; }
            set
            {
                if (this.collapsed == value)
                    return;

                this.collapsed = value;
                this.updateCollapsed();
            }
        }

        public ProjectCategory Initialised(CountedAutoCompleteView item, Action selectWithClick, string overideText = null)
        {
            this.noTaskProject.Initialised(item.View, selectWithClick, overideText);
            this.expandTasksText.Text = item.Count == 1
                ? "1 task" : string.Format("{0} tasks", item.Count);
            return this;
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            this.TaskPanel.Children.Clear();
            this.TaskPanel.Visibility = Visibility.Visible;
            this.collapsed = false;
            this.CollapsedChanged = null;
            StaticObjectPool.Push(this);
        }

        protected override void updateBackgroundColor()
        {
            base.updateBackgroundColor();

            this.noTaskProject.Background = this.Background;
        }

        private void onCollapseButtonClick(object sender, RoutedEventArgs e)
        {
            this.collapsed = !this.collapsed;

            this.updateCollapsed();

            if(this.CollapsedChanged != null)
                this.CollapsedChanged(this, EventArgs.Empty);
        }

        private void updateCollapsed()
        {
            this.TaskPanel.Visibility = this.collapsed ? Visibility.Collapsed : Visibility.Visible;
        }
    }
}
