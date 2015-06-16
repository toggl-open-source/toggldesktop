using System.Collections.Generic;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteListItem : IAutoCompleteListItem
    {
        public string Text { get; private set; }

        private bool visible;

        protected AutoCompleteListItem(string text)
        {
            this.Text = text;
            this.visible = true;
        }

        public bool Visible
        {
            get { return this.visible; }
            protected set
            {
                if (this.visible == value)
                    return;
                this.visible = value;
                if (value)
                {
                    this.show();
                }
                else
                {
                    this.hide();
                }
            }
        }

        public abstract IEnumerable<AutoCompleteItem> Complete(string input);
        public abstract IEnumerable<AutoCompleteItem> CompleteAll();
        public abstract void CreateFrameworkElement(Panel parent);

        protected abstract void hide();
        protected abstract void show();
    }
}