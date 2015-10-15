using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop
{
    public partial class TagList
    {
        public event EventHandler<string> TagRemoved;
        public event EventHandler<string> TagAdded;

        private readonly Dictionary<string, Tag> tags = new Dictionary<string, Tag>();
        private readonly Stack<string> orderedTags = new Stack<string>(); 

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
            else if(e.Key == Key.Back && !e.IsRepeat && this.textBox.CaretIndex == 0)
            {
                if (this.tryRemoveLastTag())
                    e.Handled = true;
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
            var element = TogglDesktop.Tag.Make(tag);

            this.tags.Add(tag, element);
            this.orderedTags.Push(tag);

            this.panel.Children.Insert(this.panel.Children.Count - 1, element);

            element.RemoveClicked += (s, e) =>
            {
                this.RemoveTag(tag);
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

            if (this.orderedTags.Count > 0 && this.orderedTags.Peek() == tag)
                this.orderedTags.Pop();

            element.Dispose();

            if (this.TagRemoved != null)
                this.TagRemoved(this, tag);

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
            this.orderedTags.Clear();
            if (clearTextBox)
                this.textBox.SetText("");
        }

        private bool tryRemoveLastTag()
        {
            while (this.orderedTags.Count > 0)
            {
                var tag = this.orderedTags.Pop();
                if (this.RemoveTag(tag))
                    return true;
            }

            return false;
        }

        #endregion

        public void SetKnownTags(IEnumerable<string> tags)
        {
            this.autoComplete.SetController(AutoCompleteControllers.ForTags(tags, this.Contains));
        }

        public bool Contains(string tag)
        {
            return this.tags.ContainsKey(tag);
        }

        private void autoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asStringItem = e as StringItem;
            if (asStringItem == null)
                return;

            var tag = asStringItem.Text;
            this.tryAddTag(tag);
            this.textBox.SetText("");

            if(this.autoComplete.IsOpen)
                this.autoComplete.OpenAndShowAll();
        }

        private void autoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            this.tryAddTagFromTextBox();
        }

        private void panel_OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.autoComplete.RecalculatePosition();
        }
    }
}
