
using System;
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ModelAutoCompleteItem : SimpleAutoCompleteItem<ClientAutoCompleteEntry, Toggl.Model>
    {
        public ModelAutoCompleteItem(Toggl.Model model)
            : base(model, model.Name)
        {
        }

        protected override ClientAutoCompleteEntry createElement(Action selectWithClick)
        {
            return new ClientAutoCompleteEntry(this.Item, selectWithClick);
        }
    }
}