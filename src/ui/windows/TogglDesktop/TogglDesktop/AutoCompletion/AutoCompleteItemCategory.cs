using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteItemCategory : AutoCompleteItem
    {
        public bool Collapsed
        {
            get { return this.collapsable.Collapsed; }
            set { this.collapsable.Collapsed = value; }
        }

        private readonly List<IAutoCompleteListItem> children;
        private ICollapsable collapsable;

        protected AutoCompleteItemCategory(string text, List<IAutoCompleteListItem> children)
            : base(text)
        {
            this.children = children;
        }

        public override IEnumerable<AutoCompleteItem> Complete(string[] words)
        {
            if (this.completesAll(words))
            {
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

            this.collapsable.Collapsed = false;
        }

        public override IEnumerable<AutoCompleteItem> CompleteAll()
        {
            this.Visible = true;
            this.collapsable.Collapsed = true;

            // ReSharper disable once ReturnValueOfPureMethodIsNotUsed
            // enumeration to list to make sure all visible status of all children is updated correctly
            this.children.SelectMany(c => c.CompleteAll()).ToList();

            return this.Yield();
        }

        public override IEnumerable<AutoCompleteItem> CompleteVisible()
        {
            if (!this.Visible)
                return Enumerable.Empty<AutoCompleteItem>();

            if (this.collapsable.Collapsed)
                return this.Yield();

            return this.Prepend(this.children.SelectMany(c => c.CompleteVisible()));
        }

        public override void CreateFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, AutoCompleteController controller)
        {
            var newParent = this.createFrameworkElement(parent, selectWithClick, recyclables, out this.collapsable);

            this.collapsable.CollapsedChanged += (s, e) =>
            {
                var selectThis = controller.SelectedItem != null;

                controller.SelectItem(null);
                controller.RefreshVisibleList();
                if (selectThis)
                {
                    controller.SelectItem(this);
                }
            };

            foreach (var child in this.children)
            {
                child.CreateFrameworkElement(newParent, selectWithClick, recyclables, controller);
            }
        }

        protected abstract Panel createFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, out ICollapsable collapsable
            );
    }
}