
using System;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ModelItem : SimpleItem<ModelEntry, Toggl.Model>
    {
        public ModelItem(Toggl.Model model)
            : base(model, model.Name)
        {
        }

        protected override ModelEntry createElement(Action selectWithClick)
        {
            return new ModelEntry(this.Item, selectWithClick);
        }
    }
}