
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    public partial class TagList
    {
        public event EventHandler<string> TagRemoved;
        public event EventHandler<string> TagAdded;

        private readonly Dictionary<string, Tag> tags = new Dictionary<string, Tag>();

        public int TagCount { get { return this.tags.Count; } }
        public IEnumerable<string> Tags { get { return this.tags.Keys; } }

        public TagList()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        #region controlling

        private void onMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.focusTextBox();
        }

        private void textBoxOnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Handled)
                return;

            if (this.autoComplete.IsOpen)
                return;

            // handle input if autoComplete is closed
            if (e.Key == Key.Enter || e.Key == Key.Tab)
            {
                if(this.tryAddTagFromTextBox() || e.Key == Key.Enter)
                {
                    e.Handled = true;
                }
            }
        }

        private void textBoxOnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.textBox.SetText("");
        }

        private void focusTextBox()
        {
            this.textBox.Focus();
            this.textBox.CaretIndex = this.textBox.Text.Length;
        }

        #endregion

        #region adding

        private bool tryAddTagFromTextBox()
        {
            var tag = this.textBox.Text.Trim();
            this.textBox.SetText("");
            if (string.IsNullOrWhiteSpace(tag))
                return false;
            return this.tryAddTag(tag);
        }

        private bool tryAddTag(string tag)
        {
            var success = this.AddTag(tag);
            if (success && this.TagAdded != null)
                this.TagAdded(this, tag);
            return success;
        }

        public void AddTags(IEnumerable<string> tags)
        {
            foreach (var tag in tags)
            {
                this.AddTag(tag);
            }
        }

        public bool AddTag(string tag)
        {
            if (this.tags.ContainsKey(tag))
                return false;

            this.addTag(tag);

            return true;
        }

        private void addTag(string tag)
        {
            var element = TogglDesktop.WPF.Tag.Make(tag);

            this.tags.Add(tag, element);

            this.panel.Children.Insert(this.panel.Children.Count - 1, element);

            element.RemoveClicked += (s, e) =>
            {
                this.RemoveTag(tag);
                if (this.TagRemoved != null)
                    this.TagRemoved(this, tag);
                this.focusTextBox();
            };
        }

        #endregion

        #region removing

        public bool RemoveTag(string tag)
        {
            Tag element;
            if (!this.tags.TryGetValue(tag, out element))
                return false;

            this.panel.Children.Remove(element);
            this.tags.Remove(tag);
            element.Dispose();
            return true;
        }

        public void Clear(bool clearTextBox = true)
        {
            this.panel.Children.RemoveRange(0, this.panel.Children.Count - 1);
            foreach (var tag in this.tags.Values)
            {
                tag.Dispose();
            }
            this.tags.Clear();
            if (clearTextBox)
                this.textBox.SetText("");
        }

        #endregion

        public void SetKnownTags(IEnumerable<string> tags)
        {
            this.autoComplete.SetController(AutoCompleteControllers.ForStrings(tags, this.Contains));
        }

        public bool Contains(string tag)
        {
            return this.tags.ContainsKey(tag);
        }

        private void cautoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asStringItem = e as StringItem;
            if (asStringItem == null)
                return;

            var tag = asStringItem.Text;
            this.tryAddTag(tag);
            this.textBox.SetText("");
        }

        private void autoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            this.tryAddTagFromTextBox();
        }

    }
}
