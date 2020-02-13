using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
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
        public event RoutedEventHandler ActionButtonClick;

        #endregion

        #region fields

        private ExtendedTextBox textbox;
        private ToggleButton dropDownButton;

        private bool needsToRefreshList;

        private AutoCompleteController controller;

        #endregion

        public AutoCompletionPopup()
        {
            this.DataContext = this;
            this.InitializeComponent();

            this.popup.Opened += (s, e) => this.IsOpenChanged?.Invoke(this, EventArgs.Empty);
            this.popup.Closed += (s, e) => this.IsOpenChanged?.Invoke(this, EventArgs.Empty);

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

        public static readonly DependencyProperty ActionButtonTextProperty = DependencyProperty.Register(
            "ActionButtonText", typeof(string), typeof(AutoCompletionPopup), new PropertyMetadata(default(string)));

        public string ActionButtonText
        {
            get { return (string) GetValue(ActionButtonTextProperty); }
            set { SetValue(ActionButtonTextProperty, value); }
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
                    var isKeyboardFocusWithin = popup.IsKeyboardFocusWithin;
                    if (isKeyboardFocusWithin)
                    {
                        if (listBox.IsKeyboardFocusWithin)
                        {
                            this.textbox.Focus();
                        }
                        return;
                    }
                }

                this.close();
            };
            this.popup.LostKeyboardFocus += (sender, args) =>
            {
                if (!this.textbox.IsKeyboardFocusWithin && !this.popup.IsKeyboardFocusWithin)
                {
                    this.close();
                }
            };
            this.popup.PreviewKeyDown += (sender, args) =>
            {
                if (args.Key == Key.Tab)
                {
                    if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
                    {
                        this.textbox.Focus();
                        args.Handled = true;
                    }
                    else
                    {
                        if (this.textbox.PredictFocus(FocusNavigationDirection.Down) is UIElement nextElement)
                        {
                            Keyboard.Focus(nextElement);
                            args.Handled = true;
                        }
                    }
                }
                else if (args.Key == Key.Down || args.Key == Key.Up || args.Key == Key.Escape)
                {
                    this.textbox.Focus();
                    args.Handled = true;
                }
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
            if (this.popup.IsOpen)
                this.open(true);
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
                        {
                            this.open(showAll: true);
                        }
                        else
                        {
                            this.controller.SelectNext();
                        }
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
                    {
                        if (this.IsOpen)
                        {
                            if (this.confirmCompletion(true))
                            {
                                e.Handled = true;
                                this.close();
                            }
                        }
                        return;
                    }
                case Key.Tab:
                {
                    if (this.IsOpen)
                    {
                        if (createProjectButton.IsVisible && createProjectButton.Focus())
                        {
                            e.Handled = true;
                            this.listBox.SelectedIndex = -1;
                        }
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

        private bool confirmCompletion(bool withKeyboard)
        {
            var item = this.controller.SelectedItem;
            this.select(item, withKeyboard);
            return true;
        }

        private void select(AutoCompleteItem item, bool withKeyboard)
        {
            if (withKeyboard || !this.KeepOpenWhenSelectingWithMouse)
                this.popup.IsOpen = false;

            // quick check if there is a full match with the first shown item
            if (item == null && controller.visibleItems[0].Text == this.textbox.Text)
            {
                controller.SelectNext();
                item = controller.SelectedItem;
            }

            if (item == null)
            {
                ConfirmWithoutCompletion?.Invoke(this, this.textbox.Text);
                return;
            }

            if (this.FillTextBoxOnComplete)
            {
                this.textbox.SetText(item.Text);
                this.textbox.CaretIndex = this.textbox.Text.Length;
                this.textbox.Focus();
            }

            ConfirmCompletion?.Invoke(this, item);
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

            // Reset listbox scroll position
            if (this.listBox.SelectedIndex != -1)
            {
                this.listBox.SelectedIndex = -1;
                this.listBox.UpdateLayout();
                this.listBox.ScrollIntoView(this.listBox.Items[0]);
            }

            this.ensureList();
            this.controller.Complete(showAll ? "" : this.textbox.Text);
            createProjectButton.IsEnabled = !controller.IsFullMatch;

            if (closeIfEmpty)
            {
                this.popup.IsOpen = this.controller.visibleItems.Count > 0;
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
        }

        private void ensureList()
        {
            if (!this.needsToRefreshList)
                return;

            using (Performance.Measure("building auto complete list {0}", this.controller.DebugIdentifier))
            {
                this.controller.FillList(this.listBox);
            }

            this.needsToRefreshList = false;
        }

        private void listBox_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            DependencyObject dep = (DependencyObject)e.OriginalSource;
            while ((dep != null) && !(dep is System.Windows.Controls.ListBoxItem))
            {
                if (dep is Button) return;

                dep = System.Windows.Media.VisualTreeHelper.GetParent(dep);
            }

            if (dep == null)
                return;
            int index = listBox.ItemContainerGenerator.IndexFromContainer(dep);

            e.Handled = true;
            listBox.SelectedIndex = index;

            this.confirmCompletion(false);

            // Check selected item from list if tags autocomplete
            // if (this.controller.autocompleteType == 5)
            // {
            //     IEditableCollectionView items = listBox.Items; //Cast to interface
            //     if (items.CanRemove)
            //     {
            //         items.RemoveAt(index);
            //     }
            // }
        }

        internal bool popUpOpen()
        {
            return this.popup.IsOpen;
        }

        private void CreateProjectButton_OnClick(object sender, RoutedEventArgs e)
        {
            ActionButtonClick?.Invoke(sender, e);
        }

        public bool HasKeyboardFocus() => popup.IsKeyboardFocusWithin;
    }
}

