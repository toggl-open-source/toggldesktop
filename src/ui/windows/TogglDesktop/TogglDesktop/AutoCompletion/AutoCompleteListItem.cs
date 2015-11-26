using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteListItem : IAutoCompleteListItem
    {
        public string Text { get; private set; }

        private readonly bool neverCompletes;

        private bool visible;

        protected AutoCompleteListItem(string text)
        {
            this.Text = text;
            this.neverCompletes = string.IsNullOrWhiteSpace(text);
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

        protected bool completesAll(string[] words)
        {
            return !this.neverCompletes && words.All(this.completes);
        }

        private bool completes(string word)
        {
            return this.Text.IndexOf(word, StringComparison.OrdinalIgnoreCase) >= 0;
        }


        public abstract IEnumerable<AutoCompleteItem> Complete(string[] words);
        public abstract IEnumerable<AutoCompleteItem> CompleteAll();
        public abstract IEnumerable<AutoCompleteItem> CompleteVisible();

        public abstract void CreateFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, AutoCompleteController controller
            );

        protected abstract void hide();
        protected abstract void show();
    }
}