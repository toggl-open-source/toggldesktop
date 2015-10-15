using System;

namespace TogglDesktop.AutoCompleteControls
{
    partial class StringEntry : IRecyclable
    {
        public StringEntry()
        {
            this.InitializeComponent();
        }

        public StringEntry Initialised(string item, Action selectWithClick)
        {
            this.text.Text = item;
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
