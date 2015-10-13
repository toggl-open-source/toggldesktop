using System;

namespace TogglDesktop.AutoCompleteControls
{
    partial class ModelEntry : IRecyclable
    {
        public ModelEntry()
        {
            this.InitializeComponent();
        }

        public ModelEntry Initialised(Toggl.TogglGenericView item, Action selectWithClick, string overrideText = null)
        {
            this.text.Text = overrideText ?? item.Name;
            this.setClickAction(selectWithClick);
            return this;
        }

        public void Recycle()
        {
            this.prepareForRecycling();
            StaticObjectPool.Push(this);
        }
    }
}
