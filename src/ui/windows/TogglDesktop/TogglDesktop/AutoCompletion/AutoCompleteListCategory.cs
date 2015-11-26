using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

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

        public override IEnumerable<AutoCompleteItem> Complete(string[] words)
        {
            if (this.completesAll(words))
            {
                this.Visible = true;
                return this.CompleteAll();
            }

            return this.completeRecursive(words);
        }

        private IEnumerable<AutoCompleteItem> completeRecursive(string[] input)
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

        public override IEnumerable<AutoCompleteItem> CompleteVisible()
        {
            if (this.Visible)
                return this.children.SelectMany(c => c.CompleteVisible());
            return Enumerable.Empty<AutoCompleteItem>();
        }

        public override void CreateFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, AutoCompleteController controller)
        {
            var newParent = this.createFrameworkElement(parent, recyclables);
            foreach (var child in this.children)
            {
                child.CreateFrameworkElement(newParent, selectWithClick, recyclables, controller);
            }
        }

        protected abstract Panel createFrameworkElement(Panel parent, List<IRecyclable> recyclables);
    }
}