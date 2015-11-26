using System;
using System.Windows;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.AutoCompleteControls
{
    public partial class ProjectCategory : IRecyclable, ICollapsable
    {
        private bool collapsed;

        public ProjectCategory()
        {
            this.InitializeComponent();
        }

        public ProjectCategory Initialised(CountedAutoCompleteView item, Action selectWithClick, string overideText = null)
        {
            this.noTaskProject.Initialised(item.View, selectWithClick, overideText);
            this.expandTasksButton.Content = string.Format("{0} tasks", item.Count);
            return this;
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            this.TaskPanel.Children.Clear();
            this.TaskPanel.Visibility = Visibility.Visible;
            this.collapsed = false;
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
        }

        private void updateCollapsed()
        {
            this.TaskPanel.Visibility = this.collapsed ? Visibility.Collapsed : Visibility.Visible;
        }

        public void Collapse()
        {
            this.collapsed = true;
            this.updateCollapsed();
        }

        public void Expand()
        {
            this.collapsed = false;
            this.updateCollapsed();
        }
    }
}
