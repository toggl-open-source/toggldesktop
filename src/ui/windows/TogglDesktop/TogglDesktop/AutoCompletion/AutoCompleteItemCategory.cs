using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteItemCategory : AutoCompleteItem
    {
        private readonly List<IAutoCompleteListItem> children;

        protected AutoCompleteItemCategory(string text, List<IAutoCompleteListItem> children)
            : base(text)
        {
            this.children = children;
        }

        public override IEnumerable<AutoCompleteItem> Complete(string[] words)
        {
            if (this.completesAll(words))
            {
                this.Visible = true;

                // todo: collapse children

                return this.CompleteAll();
            }

            return this.completeRecursive(words);
        }

        private IEnumerable<AutoCompleteItem> completeRecursive(string[] input)
        {
            var anyChildren = false;
            foreach (var descendent in this.children.SelectMany(c => c.Complete(input)))
            {
                if (!anyChildren)
                {
                    yield return this;
                }

                anyChildren = true;
                yield return descendent;
            }
            this.Visible = anyChildren;

            if (anyChildren)
            {
                // todo: expand children
            }
        }

        public override IEnumerable<AutoCompleteItem> CompleteAll()
        {
            this.Visible = true;
            return this.Prepend(this.children.SelectMany(c => c.CompleteAll()));
        }

        public override void CreateFrameworkElement(Panel parent, Action<AutoCompleteItem> selectWithClick, List<IRecyclable> recyclables)
        {
            var newParent = this.createFrameworkElement(parent, selectWithClick, recyclables);
            foreach (var child in this.children)
            {
                child.CreateFrameworkElement(newParent, selectWithClick, recyclables);
            }
        }

        protected abstract Panel createFrameworkElement(Panel parent, Action<AutoCompleteItem> selectWithClick, List<IRecyclable> recyclables);
    }
}