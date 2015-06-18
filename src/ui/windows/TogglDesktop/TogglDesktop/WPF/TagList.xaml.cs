
using System;
using System.Collections.Generic;
using System.Windows.Input;

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

        private void onMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.focusTextBox();
        }

        private void textBoxOnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter || e.Key == Key.Tab)
            {
                var tag = this.textBox.Text.Trim();
                if (!string.IsNullOrWhiteSpace(tag))
                {
                    if (this.AddTag(tag) && this.TagAdded != null)
                        this.TagAdded(this, tag);
                    e.Handled = true;
                }
                else if(e.Key == Key.Enter)
                {
                    e.Handled = true;
                }
                this.textBox.Text = "";
            }
        }

        private void focusTextBox()
        {
            this.textBox.Focus();
            this.textBox.CaretIndex = this.textBox.Text.Length;
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
            var element = new Tag(tag);

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

        public bool RemoveTag(string tag)
        {
            Tag element;
            if (!this.tags.TryGetValue(tag, out element))
                return false;

            this.panel.Children.Remove(element);
            this.tags.Remove(tag);
            return true;
        }

        public void Clear(bool clearTextBox = true)
        {
            this.panel.Children.RemoveRange(0, this.panel.Children.Count - 1);
            this.tags.Clear();
            if (clearTextBox)
                this.textBox.Text = "";
        }

    }
}
