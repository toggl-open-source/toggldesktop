using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using MahApps.Metro.Controls;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop
{
    public partial class TagList
    {
        public event EventHandler<string> TagRemoved;
        public event EventHandler<string> TagAdded;

        private readonly Dictionary<string, Tag> _tags = new Dictionary<string, Tag>();
        private readonly Stack<string> _orderedTags = new Stack<string>();
        private TagsAutoCompleteController _controller;

        public int TagCount => this._tags.Count;
        public IEnumerable<string> Tags => this._tags.Keys;

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
                if (this.tryAddTagFromTextBox() || e.Key == Key.Enter)
                {
                    e.Handled = true;
                }
            }
            else if (e.Key == Key.Back && !e.IsRepeat && this.textBox.CaretIndex == 0)
            {
                if (this.tryRemoveLastTag())
                    e.Handled = true;
            }
        }

        private void textBoxOnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (autoComplete.HasKeyboardFocus())
            {
                return;
            }

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
            if (success)
                TagAdded?.Invoke(this, tag);
            return success;
        }

        private bool tryAddOrRemoveTag(string tag)
        {
            return tryAddTag(tag) || RemoveTag(tag);
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
            if (this._tags.ContainsKey(tag))
                return false;

            this.addTag(tag);

            return true;
        }

        private void addTag(string tag)
        {
            var element = TogglDesktop.Tag.Make(tag);

            this._tags.Add(tag, element);
            this._orderedTags.Push(tag);
            this._controller?.AddTag(tag);

            this.panel.Children.Insert(this.panel.Children.Count - 1, element);

            element.RemoveClicked += (s, e) =>
            {
                this.RemoveTag(tag);
                this.focusTextBox();
            };

            ClearEmptyText();
        }

        #endregion

        #region removing

        public bool RemoveTag(string tag)
        {
            if (!this._tags.TryGetValue(tag, out Tag element))
                return false;

            this.panel.Children.Remove(element);
            this._tags.Remove(tag);
            this._controller?.RemoveTag(tag);

            if (this._orderedTags.Count > 0 && this._orderedTags.Peek() == tag)
                this._orderedTags.Pop();

            element.Dispose();

            if (_tags.Count == 0)
                this.SetEmptyText();

            TagRemoved?.Invoke(this, tag);

            return true;
        }

        public void Clear(bool clearTextBox = true)
        {
            this.panel.Children.RemoveRange(0, this.panel.Children.Count - 1);
            foreach (var tag in this._tags.Values)
            {
                tag.Dispose();
            }

            this._tags.Clear();
            this._orderedTags.Clear();
            this._controller.ClearSelection();
            if (clearTextBox)
                this.textBox.SetText("");

            this.SetEmptyText();
        }

        private bool tryRemoveLastTag()
        {
            while (this._orderedTags.Count > 0)
            {
                var tag = this._orderedTags.Pop();
                if (this.RemoveTag(tag))
                    return true;
            }

            return false;
        }

        #endregion

        public void SetKnownTags(IEnumerable<string> tags)
        {
            if (!autoComplete.IsOpen)
            {
                _controller = AutoCompleteControllersFactory.ForTags(tags, this._tags.ContainsKey);
                this.autoComplete.SetController(_controller);
            }
        }

        public void ClearUndoHistory()
        {
            this.textBox.ClearUndoHistory();
        }

        private void ClearEmptyText() => this.textBox.SetValue(TextBoxHelper.WatermarkProperty, string.Empty);
        private void SetEmptyText() => this.textBox.SetValue(TextBoxHelper.WatermarkProperty, "Add tags");

        private void autoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            if (e is IModelItem<string> asStringItem)
            {
                var tag = asStringItem.Text;
                this.tryAddOrRemoveTag(tag);
                this.textBox.SetText("");

                if (this.autoComplete.IsOpen)
                    this.autoComplete.OpenAndShowAll();
            }
        }

        private void autoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            this.tryAddTagFromTextBox();
        }

        private void panel_OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.autoComplete.RecalculatePosition();
        }

        private void AutoComplete_OnActionButtonClick(object sender, RoutedEventArgs e)
        {
            this.tryAddTagFromTextBox();
        }

        private void AutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            if (!autoComplete.IsOpen)
            {
                if (!this.textBox.Text.IsNullOrEmpty())
                {
                    this.textBox.SetText("");
                }
            }
        }
    }
}
