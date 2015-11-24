

using System;

namespace TogglDesktop.AutoCompleteControls
{
    public partial class ProjectCategory : IRecyclable
    {
        public ProjectCategory()
        {
            this.InitializeComponent();
        }

        public ProjectCategory Initialised(Toggl.TogglAutocompleteView item, Action selectWithClick, string overideText = null)
        {
            this.noTaskProject.Initialised(item, selectWithClick, overideText);
            return this;
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            StaticObjectPool.Push(this);
        }
    }
}
