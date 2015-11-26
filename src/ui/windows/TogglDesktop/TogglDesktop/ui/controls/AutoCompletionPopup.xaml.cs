using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using TogglDesktop.AutoCompleteControls;
using TogglDesktop.AutoCompletion;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    partial class AutoCompletionPopup
    {
        #region events

        public event EventHandler<AutoCompleteItem> ConfirmCompletion;
        public event EventHandler<string> ConfirmWithoutCompletion;

        public event EventHandler IsOpenChanged;

        #endregion

        #region fields

        private ExtendedTextBox textbox;
        private ToggleButton dropDownButton;

        private bool needsToRefreshList;

        private AutoCompleteController controller;

        private readonly List<IRecyclable> recyclableEntries = new List<IRecyclable>();

        #endregion

        public AutoCompletionPopup()
        {
            this.DataContext = this;
            this.InitializeComponent();

            this.popup.Opened += (s, e) => this.tryInvoke(this.IsOpenChanged);
            this.popup.Closed += (s, e) => this.tryInvoke(this.IsOpenChanged);

            this.IsEnabledChanged += this.onIsEnabledChanged;

            this.FillTextBoxOnComplete = true;
        }

        #region properties

        public bool IsOpen
        {
            get { return this.popup.IsOpen; }
            set
            {
                if (value)
                    this.open();
                else
                    this.close();
            }
        }

        public bool StaysOpen
        {
            get { return this.popup.StaysOpen; }
            set { this.popup.StaysOpen = value; }
        }

        public bool KeepOpenWhenSelectingWithMouse { get; set; }
        public bool FillTextBoxOnComplete { get; set; }

        public string EmptyText
        {
            get { return this.emptyLabel.Text; }
            set { this.emptyLabel.Text = value; }
        }

        #endregion

        #region dependency properties

        #region Target

        public static readonly DependencyProperty TargetProperty = DependencyProperty
            .Register("Target", typeof (FrameworkElement), typeof (AutoCompletionPopup),
                new FrameworkPropertyMetadata
                {
                    PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).updateTarget()
                });

        public FrameworkElement Target
        {
            get { return (FrameworkElement)this.GetValue(TargetProperty); }
            set { this.SetValue(TargetProperty, value); }
        }

        #endregion

        #region TextBox

        public static readonly DependencyProperty TextBoxProperty = DependencyProperty
            .Register("TextBox", typeof (ExtendedTextBox), typeof (AutoCompletionPopup),
                new FrameworkPropertyMetadata
                {
                    PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).initialise()
                });

        public ExtendedTextBox TextBox
        {
            get { return (ExtendedTextBox)this.GetValue(TextBoxProperty); }
            set { this.SetValue(TextBoxProperty, value); }
        }

        #endregion

        #region DropDownButton

        public static readonly DependencyProperty DropDownButtonProperty = DependencyProperty
           .Register("DropDownButton", typeof(ToggleButton), typeof(AutoCompletionPopup),
           new FrameworkPropertyMetadata
           {
               PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).initDropDownButton()
           });

        public ToggleButton DropDownButton
        {
            get { return (ToggleButton)this.GetValue(DropDownButtonProperty); }
            set { this.SetValue(DropDownButtonProperty, value); }
        }

        #endregion

        #endregion

        #region setup

        private void initialise()
        {
            if (DesignerProperties.GetIsInDesignMode(this))
                return;

            if (this.textbox != null)
                throw new Exception("Auto completion popup cannot be initialised more than once.");

            this.textbox = this.TextBox;

            if (this.textbox == null)
                throw new Exception("Auto completion popup must have a valid text box.");

            this.textbox.PreviewKeyDown += this.textboxOnPreviewKeyDown;
            this.textbox.TextChanged += this.textboxOnTextChanged;
            this.textbox.LostKeyboardFocus += (sender, args) =>
            {
                if (this.textbox.Focusable && this.textbox.IsEnabled)
                {
                    var element = Keyboard.FocusedElement as FrameworkElement;
                    while (true)
                    {
                        if (element == null)
                            break;

                        if (element == this)
                        {
                            this.textbox.Focus();
                            return;
                        }

                        element = element.Parent as FrameworkElement;
                    }
                }

                this.close();
            };
        }

        private void initDropDownButton()
        {
            if (DesignerProperties.GetIsInDesignMode(this))
                return;

            if (this.dropDownButton != null)
                throw new Exception("Cannot set auto completion drop down button more than once.");

            this.dropDownButton = this.DropDownButton;

            if (this.dropDownButton == null)
                throw new Exception("Cannot set auto completion drop down button to null.");

            this.IsOpenChanged += this.updateDropDownButton;
            this.dropDownButton.Click += this.onDropDownButtonClick;
        }

        #endregion

        public void SetController(AutoCompleteController controller)
        {
            this.controller = controller;
            this.needsToRefreshList = true;
        }

        public void OpenAndShowAll()
        {
            this.open(showAll: true);
            this.textbox.SelectAll();
            this.textbox.Focus();
        }

        public void RecalculatePosition()
        {
            if (!this.popup.IsOpen)
                return;

            this.updateTarget();
            // hack to make the popup re-calculate its position
            var offset = this.popup.HorizontalOffset;
            this.popup.HorizontalOffset = offset + 1;
            this.popup.HorizontalOffset = offset;
        }

        #region ui events and overrides

        private void onIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs args)
        {
            if (this.IsEnabled)
                return;

            this.close();
        }

        private void textboxOnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.IsEnabled || e.Handled)
                return;

            switch (e.Key)
            {
                case Key.Down:
                    {
                        if (!this.IsOpen)
                            this.open(showAll: true);
                        this.controller.SelectNext();
                        e.Handled = true;
                        return;
                    }
                case Key.Up:
                    {
                        if (this.IsOpen)
                            this.controller.SelectPrevious();
                        e.Handled = true;
                        return;
                    }
                case Key.Left:
                    {
                        if (this.IsOpen)
                        {
                            if (this.controller.TryCollapseCategory())
                                e.Handled = true;
                        }
                        return;
                    }
                case Key.Right:
                    {
                        if (this.IsOpen)
                        {
                            if (this.controller.TryExpandCategory())
                                e.Handled = true;
                        }
                        return;
                    }
                case Key.Escape:
                    {
                        if (this.IsOpen)
                        {
                            this.close();
                            e.Handled = true;
                        }
                        return;
                    }
                case Key.Enter:
                case Key.Tab:
                    {
                        if (this.IsOpen)
                        {
                            this.confirmCompletion();
                            e.Handled = true;
                        }
                        return;
                    }
            }
        }

        private void textboxOnTextChanged(object sender, TextChangedEventArgs e)
        {
            if (!this.IsEnabled)
                return;

            if (this.textbox.IsTextChangingProgrammatically)
                return;

            this.open(true);
        }

        private void onDropDownButtonClick(object s, RoutedEventArgs e)
        {
            var open = this.dropDownButton.IsChecked ?? false;
            if (open)
            {
                this.OpenAndShowAll();
            }
            else
            {
                this.close();
                if (!this.textbox.IsKeyboardFocused)
                {
                    this.textbox.Focus();
                    this.textbox.CaretIndex = this.textbox.Text.Length;
                }
            }
        }

        private void updateDropDownButton(object sender, EventArgs e)
        {
            this.dropDownButton.IsChecked = this.IsOpen;
        }

        #endregion

        private void confirmCompletion()
        {
            var item = this.controller.SelectedItem;

            this.select(item, true);
        }

        private void select(AutoCompleteItem item)
        {
            this.select(item, false);
        }

        private void select(AutoCompleteItem item, bool withKeyboard)
        {
            if (withKeyboard || !this.KeepOpenWhenSelectingWithMouse)
                this.popup.IsOpen = false;

            if (item == null)
            {
                if (this.ConfirmWithoutCompletion != null)
                    this.ConfirmWithoutCompletion(this, this.textbox.Text);
                return;
            }

            if (this.FillTextBoxOnComplete)
            {
                this.textbox.SetText(item.Text);
                this.textbox.CaretIndex = this.textbox.Text.Length;
                this.textbox.Focus();
            }

            if (this.ConfirmCompletion != null)
                this.ConfirmCompletion(this, item);
        }

        private void close()
        {
            this.popup.IsOpen = false;
        }

        private void open(bool closeIfEmpty = false, bool showAll = false)
        {
            if (!showAll && this.textbox.Text == "" && !this.popup.IsOpen)
            {
                return;
            }

            if (!this.popup.IsOpen)
            {
                this.updateTarget();
            }

            // fix to make sure list updates layout when first opened
            this.popup.IsOpen = true;

            this.ensureList();
            this.controller.Complete(showAll ? "" : this.textbox.Text);
            this.emptyLabel.ShowOnlyIf(this.controller.VisibleItems.Count == 0);

            if (closeIfEmpty)
            {
                this.popup.IsOpen = this.controller.VisibleItems.Count > 0;   
            }
            else
            {
                this.popup.IsOpen = true;
            }
        }

        private void updateTarget()
        {
            var target = this.Target;
            this.popup.PlacementTarget = target;
            this.popup.MinWidth = target == null ? 0 : target.ActualWidth + 20;
        }

        private void ensureList()
        {
            if (!this.needsToRefreshList)
                return;

            using (Performance.Measure("building auto complete list {0}", this.controller.DebugIdentifier))
            {
                this.dropDownList.Children.Clear();
                if (this.recyclableEntries.Count > 0)
                {
                    using (Performance.Measure("recycling entries, count: " + this.recyclableEntries.Count))
                    {
                        foreach (var entry in this.recyclableEntries)
                        {
                            entry.Recycle();
                        }
                        this.recyclableEntries.Clear();
                    }
                }
                this.controller.FillList(this.dropDownList, this.select, this.recyclableEntries);
            }

            this.needsToRefreshList = false;
        }

        private void tryInvoke(EventHandler e)
        {
            if (e != null)
                e(this, EventArgs.Empty);
        }

    }
}
