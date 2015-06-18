
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ClientAutoCompleteItem : SimpleAutoCompleteItem<ClientAutoCompleteEntry, Toggl.Model>
    {
        public ClientAutoCompleteItem(Toggl.Model model)
            : base(model, model.Name)
        {
        }

        protected override ClientAutoCompleteEntry createElement()
        {
            return new ClientAutoCompleteEntry(this.Item);
        }
    }
}