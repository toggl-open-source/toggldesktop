using System;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.AutoCompleteControls
{
    public partial class ProjectCategory : IRecyclable
    {
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
            StaticObjectPool.Push(this);
        }

        protected override void updateBackgroundColor()
        {
            base.updateBackgroundColor();

            this.noTaskProject.Background = this.Background;
        }
    }
}
