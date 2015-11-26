using System.Collections.Generic;

namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteItem : AutoCompleteListItem
    {
        private bool selected;

        protected AutoCompleteItem(string text)
            : base(text)
        {
        }

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                this.selected = value;
                if (value)
                {
                    this.select();
                }
                else
                {
                    this.unselect();
                }
            }
        }


        public override IEnumerable<AutoCompleteItem> Complete(string[] words)
        {
            this.Visible = this.completesAll(words);

            if (this.Visible)
            {
                yield return this;
            }
        }

        public override IEnumerable<AutoCompleteItem> CompleteAll()
        {
            this.Visible = true;
            yield return this;
        }

        public override IEnumerable<AutoCompleteItem> CompleteVisible()
        {
            if (this.Visible)
                yield return this;
        }

        protected abstract void select();
        protected abstract void unselect();

    }
}