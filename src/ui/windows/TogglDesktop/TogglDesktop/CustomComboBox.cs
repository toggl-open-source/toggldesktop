using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace TogglDesktop
{
    public class CustomComboBox : TextBox
    {
        public ListBox autoCompleteListBox;
        private bool _isAdded;
        private String _formerValue = String.Empty;
        public bool fullListOpened = false;
        private SizeF currentFactor;
        private int defaultItemHeight;
        private bool mouseEntered = false;

        public CustomComboBox()
        {
            InitializeComponent();
            defaultItemHeight = autoCompleteListBox.ItemHeight;
        }

        private void InitializeComponent()
        {
            autoCompleteListBox = new ListBox();
            autoCompleteListBox.Visible = false;
            autoCompleteListBox.DrawMode = DrawMode.OwnerDrawFixed;
            autoCompleteListBox.DrawItem += autoCompleteListBox_DrawItem;
            autoCompleteListBox.MouseEnter += autoCompleteListBox_MouseEnter;
            autoCompleteListBox.MouseLeave += autoCompleteListBox_MouseLeave;
            autoCompleteListBox.MouseWheel += autoCompleteListBox_MouseWheel;
            MouseWheel += autoCompleteListBox_MouseWheel;
            SizeChanged += CustomComboBox_SizeChanged;
            autoCompleteListBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
            | System.Windows.Forms.AnchorStyles.Right | System.Windows.Forms.AnchorStyles.Bottom )));

        }

        private void CustomComboBox_SizeChanged(object sender, EventArgs e)
        {
            if (Parent == null) {
                return;
            }
            autoCompleteListBox.MaximumSize = new Size(Width + 10, Parent.Height - Height - Top);
            autoCompleteListBox.Width = Width;
        }

        void autoCompleteListBox_MouseLeave(object sender, EventArgs e)
        {
            mouseEntered = false;
        }

        void autoCompleteListBox_MouseWheel(object sender, MouseEventArgs e)
        {
            if (mouseEntered)
            {
                autoCompleteListBox.Focus();
                mouseEntered = false;
            }
        }

        void autoCompleteListBox_MouseEnter(object sender, EventArgs e)
        {
            mouseEntered = true;
        }

        protected override void ScaleControl(SizeF factor, BoundsSpecified specified)
        {
            base.ScaleControl(factor, specified);
            currentFactor = factor;
        }

        private void autoCompleteListBox_DrawItem(object sender, DrawItemEventArgs e)
        {
            autoCompleteListBox.ItemHeight = (int)(defaultItemHeight*currentFactor.Height);
            e.DrawBackground();
            e.DrawFocusRectangle();
            e.Graphics.DrawString(
                 autoCompleteListBox.Items[e.Index].ToString(),
                 e.Font,
                 new SolidBrush(e.ForeColor),
                 e.Bounds);
        }

        public void InitListBox()
        {
            if (!_isAdded)
            {
                Parent.Controls.Add(autoCompleteListBox);
                autoCompleteListBox.Left = Left;
                autoCompleteListBox.Top = Top + Height;
                _isAdded = true;
            }
        }

        public void ShowListBox()
        {
            autoCompleteListBox.MaximumSize = new Size(Width + 10, Parent.Height - Height - Top);
            autoCompleteListBox.Visible = true;
            autoCompleteListBox.BringToFront();
        }

        public void ResetListBox()
        {
            autoCompleteListBox.Visible = false;
            fullListOpened = false;
            mouseEntered = false;
        }

        public void UpdateListBox(List<Toggl.AutocompleteItem> autoCompleteList,
            KeyEventArgs e)
        {
            if (Text == _formerValue)
            {
                return;
            }

            _formerValue = Text;
            String word = Text;

            ResetListBox();

            if (null == autoCompleteList || word.Length == 0)
            {
                return;
            }

            autoCompleteListBox.Items.Clear();

            foreach (Toggl.AutocompleteItem item in autoCompleteList)
            {
                if (item.ToString().IndexOf(word, StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    autoCompleteListBox.Items.Add(item);
                }
            }

            if (autoCompleteListBox.Items.Count == 0)
            {
                ResetListBox();
                return;
            }

            InitListBox();

            autoCompleteListBox.SelectedIndex = 0;
            autoCompleteListBox.Height = 0;
            autoCompleteListBox.Width = 0;

            Focus();
            int maxWidth = 0;
            using (Graphics graphics = autoCompleteListBox.CreateGraphics())
            {
                for (int i = 0; i < autoCompleteListBox.Items.Count; i++)
                {
                    autoCompleteListBox.Height += autoCompleteListBox.GetItemHeight(i);
                    // if item width is larger than the current one
                    // set it to the new max item width
                    // GetItemRectangle does not work for me
                    // we add a little extra space by using '_'
                    int itemWidth = (int)graphics.MeasureString((autoCompleteListBox.Items[i].ToString()) + "_", autoCompleteListBox.Font).Width;
                    maxWidth = (maxWidth < itemWidth) ? itemWidth : maxWidth;
                }
                autoCompleteListBox.Width = Math.Max(maxWidth, Width);
            }

            // Don't show the listbox again, when Enter was pressed
            // (which indicates selection was made)
            if (e.KeyCode == Keys.Enter)
            {
                return;
            }

            ShowListBox();
        }

        public Boolean parseKeyDown(PreviewKeyDownEventArgs e, List<Toggl.AutocompleteItem> autoCompleteList) 
        {
            switch (e.KeyCode)
            {
                case Keys.Enter:
                    {
                        if (autoCompleteList == null)
                        {
                            return false;
                        }
                        if (autoCompleteListBox.Visible)
                        {
                            //ResetListBox();
                            _formerValue = Text;
                            return false;
                        }

                        break;
                    }
                case Keys.Tab:
                    {
                        if (autoCompleteListBox.Visible)
                        {
                            ResetListBox();
                            _formerValue = Text;
                            return true;
                        }
                        break;
                    }
                case Keys.Down:
                    {
                        autoCompleteListBox.Focus();
                        if ((autoCompleteListBox.Visible) && (autoCompleteListBox.SelectedIndex < autoCompleteListBox.Items.Count - 1))
                            autoCompleteListBox.SelectedIndex++;

                        break;
                    }
                case Keys.Up:
                    {
                        if ((autoCompleteListBox.Visible) && (autoCompleteListBox.SelectedIndex > 0))
                            autoCompleteListBox.SelectedIndex--;

                        break;
                    }
            }
            return false;
        }

        internal void openFullList(List<Toggl.AutocompleteItem> autoCompleteList)
        {
            int maxWidth = 0;
            ResetListBox();
            autoCompleteListBox.Items.Clear();
            foreach (Toggl.AutocompleteItem item in autoCompleteList)
            {
                autoCompleteListBox.Items.Add(item);
            }
            if (autoCompleteListBox.Items.Count > 0)
            {
                InitListBox();
                autoCompleteListBox.SelectedIndex = 0;
                autoCompleteListBox.Height = 0;
                autoCompleteListBox.Width = 0;
                Focus();
                using (Graphics graphics = autoCompleteListBox.CreateGraphics())
                {
                    for (int i = 0; i < autoCompleteListBox.Items.Count; i++)
                    {
                        autoCompleteListBox.Height += autoCompleteListBox.GetItemHeight(i);
                        // it item width is larger than the current one
                        // set it to the new max item width
                        // GetItemRectangle does not work for me
                        // we add a little extra space by using '_'
                        int itemWidth = (int)graphics.MeasureString((autoCompleteListBox.Items[i].ToString()) + "_", autoCompleteListBox.Font).Width;
                        maxWidth = (maxWidth < itemWidth) ? itemWidth : maxWidth;
                    }
                    autoCompleteListBox.Width = Math.Max(maxWidth, Width);                   
                }
                ShowListBox();
            }
            fullListOpened = true;
        }

        internal void handleLeave()
        {
            if (!autoCompleteListBox.Focused)
            {
                ResetListBox();
            }
        }
    }
}
