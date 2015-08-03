
using System;
using System.Collections.Generic;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class NoClientItem : ModelItem
    {
        public NoClientItem()
            : base(new Toggl.Model())
        {
        }

        public override IEnumerable<AutoCompleteItem> Complete(string[] input)
        {
            yield return this;
        }

        protected override ModelEntry createElement(Action selectWithClick)
        {
            return new ModelEntry(this.Item, selectWithClick, "No client");
        }
    }

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