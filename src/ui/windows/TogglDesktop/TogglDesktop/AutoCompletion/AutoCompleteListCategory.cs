using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteListCategory : AutoCompleteListItem
    {
        private readonly List<IAutoCompleteListItem> children;

        protected AutoCompleteListCategory(string text, List<IAutoCompleteListItem> children)
            : base(text)
        {
            this.children = children;
        }

        public override IEnumerable<AutoCompleteItem> Complete(string input)
        {
            if (this.Text.Contains(input))
            {
                this.Visible = true;
                return this.CompleteAll();
            }

            return this.completeRecursive(input);
        }

        private IEnumerable<AutoCompleteItem> completeRecursive(string input)
        {
            var anyChildren = false;
            foreach (var descendent in this.children.SelectMany(c => c.Complete(input)))
            {
                anyChildren = true;
                yield return descendent;
            }
            this.Visible = anyChildren;
        }

        public override IEnumerable<AutoCompleteItem> CompleteAll()
        {
            this.Visible = true;
            return this.children.SelectMany(c => c.CompleteAll());
        }

        public override void CreateFrameworkElement(Panel parent, Action<AutoCompleteItem> selectWithClick)
        {
            var newParent = this.createFrameworkElement(parent);
            foreach (var child in this.children)
            {
                child.CreateFrameworkElement(newParent, selectWithClick);
            }
        }

        protected abstract Panel createFrameworkElement(Panel parent);
    }
}