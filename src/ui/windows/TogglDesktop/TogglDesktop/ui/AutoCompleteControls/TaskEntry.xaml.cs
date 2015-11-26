using System;

namespace TogglDesktop.AutoCompleteControls
{
    partial class TaskEntry : IRecyclable
    {
        public TaskEntry()
        {
            this.InitializeComponent();
        }

        public TaskEntry Initialised(Toggl.TogglAutocompleteView item, Action selectWithClick)
        {
            this.task.Text = item.TaskLabel;
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
