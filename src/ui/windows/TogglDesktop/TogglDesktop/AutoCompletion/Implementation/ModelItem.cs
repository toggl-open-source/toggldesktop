
using System;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ModelItem : SimpleItem<ClientEntry, Toggl.Model>
    {
        public ModelItem(Toggl.Model model)
            : base(model, model.Name)
        {
        }

        protected override ClientEntry createElement(Action selectWithClick)
        {
            return new ClientEntry(this.Item, selectWithClick);
        }
    }
}