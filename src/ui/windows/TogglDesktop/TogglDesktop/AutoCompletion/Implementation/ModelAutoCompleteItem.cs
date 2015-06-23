
using System;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ModelAutoCompleteItem : SimpleAutoCompleteItem<ClientEntry, Toggl.Model>
    {
        public ModelAutoCompleteItem(Toggl.Model model)
            : base(model, model.Name)
        {
        }

        protected override ClientEntry createElement(Action selectWithClick)
        {
            return new ClientEntry(this.Item, selectWithClick);
        }
    }
}