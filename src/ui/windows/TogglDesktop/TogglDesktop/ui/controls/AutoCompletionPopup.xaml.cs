using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    internal partial class AutoCompletionPopup
    {
        #region events

        public event EventHandler<IAutoCompleteItem> ConfirmCompletion;
        public event EventHandler<string> ConfirmWithoutCompletion;
        public event EventHandler IsOpenChanged;
        public event RoutedEventHandler ActionButtonClick;

        #endregion

        #region fields

        private ExtendedTextBox _textbox;
        private ToggleButton _dropDownButton;

        private bool _needsToRefreshList;

        private IAutoCompleteController _controller;

        #endregion

        public AutoCompletionPopup()
        {
            this.DataContext = this;
            this.InitializeComponent();

            this.popup.Opened += (s, e) => this.IsOpenChanged?.Invoke(this, EventArgs.Empty);
            this.popup.Closed += (s, e) => this.IsOpenChanged?.Invoke(this, EventArgs.Empty);

            this.IsEnabledChanged += this.onIsEnabledChanged;
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

        public bool KeepOpenWhenSelecting { get; set; }

        #endregion

        #region dependency properties

        #region Target

        public static readonly DependencyProperty TargetProperty = DependencyProperty
            .Register("Target", typeof(FrameworkElement), typeof(AutoCompletionPopup),
                new FrameworkPropertyMetadata
                {
                    PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).updateTarget(),
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
            get { return (string)GetValue(ActionButtonTextProperty); }
            set { SetValue(ActionButtonTextProperty, value); }
        }

        #endregion

        #region TextBox

        public static readonly DependencyProperty TextBoxProperty = DependencyProperty
            .Register("TextBox", typeof(ExtendedTextBox), typeof(AutoCompletionPopup),
                new FrameworkPropertyMetadata
                {
                    PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).initialise(),
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
               PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).initDropDownButton(),
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

            if (this._textbox != null)
                throw new Exception("Auto completion popup cannot be initialised more than once.");

            this._textbox = this.TextBox;

            if (this._textbox == null)
                throw new Exception("Auto completion popup must have a valid text box.");

            this._textbox.PreviewKeyDown += this.textboxOnPreviewKeyDown;
            this._textbox.TextChanged += this.textboxOnTextChanged;
            this._textbox.LostKeyboardFocus += (sender, args) =>
            {
                if (this._textbox.Focusable && this._textbox.IsEnabled)
                {
                    var isKeyboardFocusWithin = popup.IsKeyboardFocusWithin;
                    if (isKeyboardFocusWithin)
                    {
                        if (listBox.IsKeyboardFocusWithin)
                        {
                            this._textbox.Focus();
                        }

                        return;
                    }
                }

                this.close();
            };
            this.popup.LostKeyboardFocus += (sender, args) =>
            {
                if (!this._textbox.IsKeyboardFocusWithin && !this.popup.IsKeyboardFocusWithin)
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
                        this._textbox.Focus();
                        args.Handled = true;
                    }
                    else
                    {
                        if (this._textbox.PredictFocus(FocusNavigationDirection.Down) is UIElement nextElement)
                        {
                            Keyboard.Focus(nextElement);
                            args.Handled = true;
                        }
                    }
                }
                else if (args.Key == Key.Down || args.Key == Key.Up || args.Key == Key.Escape)
                {
                    this._textbox.Focus();
                    args.Handled = true;
                }
            };
        }

        private void initDropDownButton()
        {
            if (DesignerProperties.GetIsInDesignMode(this))
                return;

            if (this._dropDownButton != null)
                throw new Exception("Cannot set auto completion drop down button more than once.");

            this._dropDownButton = this.DropDownButton;

            if (this._dropDownButton == null)
                throw new Exception("Cannot set auto completion drop down button to null.");

            this.IsOpenChanged += this.updateDropDownButton;
            this._dropDownButton.Click += this.onDropDownButtonClick;
        }

        #endregion

        public void SetController(IAutoCompleteController controller)
        {
            this._controller = controller;
            this._needsToRefreshList = true;
            if (this.popup.IsOpen)
                this.open(true);
        }

        public void OpenAndShowAll()
        {
            this.open(showAll: true);
            this._textbox.SelectAll();
            this._textbox.Focus();
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
                            this._controller.SelectNext();
                        }

                        e.Handled = true;
                        return;
                    }

                case Key.Up:
                    {
                        if (this.IsOpen)
                            this._controller.SelectPrevious();
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
                            }
                        }

                        return;
                    }

                case Key.Tab:
                    {
                        if (this.IsOpen)
                        {
                            if (actionButton.IsVisible && actionButton.Focus())
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

            if (this._textbox.IsTextChangingProgrammatically)
                return;

            this.open(true);
        }

        private void onDropDownButtonClick(object s, RoutedEventArgs e)
        {
            var open = this._dropDownButton.IsChecked ?? false;
            if (open)
            {
                this.OpenAndShowAll();
            }
            else
            {
                this.close();
                if (!this._textbox.IsKeyboardFocused)
                {
                    this._textbox.Focus();
                    this._textbox.CaretIndex = this._textbox.Text.Length;
                }
            }
        }

        private void updateDropDownButton(object sender, EventArgs e)
        {
            this._dropDownButton.IsChecked = this.IsOpen;
        }

        #endregion

        private bool confirmCompletion(bool withKeyboard)
        {
            var item = this._controller.SelectedItem;
            this.select(item, withKeyboard);
            return true;
        }

        private void select(IAutoCompleteItem item, bool withKeyboard)
        {
            if (!this.KeepOpenWhenSelecting)
                this.close();

            if (item == null)
            {
                ConfirmWithoutCompletion?.Invoke(this, this._textbox.Text);
                if (this.IsOpen)
                {
                    // refresh the popup content
                    this.open();
                }

                return;
            }

            ConfirmCompletion?.Invoke(this, item);
        }

        private void close()
        {
            this.popup.IsOpen = false;
        }

        private void open(bool closeIfEmpty = false, bool showAll = false)
        {
            if (!showAll && this._textbox.Text == "" && !this.popup.IsOpen)
            {
                return;
            }

            if (!this.popup.IsOpen)
            {
                this.updateTarget();
            }

            // fix to make sure list updates layout when first opened
            this.popup.IsOpen = true;

            if (!KeepOpenWhenSelecting)
            {
                // Reset listbox scroll position
                if (this.listBox.SelectedIndex != -1)
                {
                    this.listBox.SelectedIndex = -1;
                    this.listBox.UpdateLayout();
                    this.listBox.ScrollIntoView(this.listBox.Items[0]);
                }
            }

            this.ensureList();
            this._controller.Complete(showAll ? "" : this._textbox.Text);
            this.actionButton.ShowOnlyIf(this._controller.ShowActionButton && !ActionButtonText.IsNullOrEmpty());

            if (closeIfEmpty)
            {
                this.popup.IsOpen = this._controller.VisibleItems.Count > 0;
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
            if (!this._needsToRefreshList)
                return;

            using (Performance.Measure("building auto complete list {0}", this._controller.DebugIdentifier))
            {
                this._controller.FillList(this.listBox);
                this.actionButton.ShowOnlyIf(this._controller.ShowActionButton && !ActionButtonText.IsNullOrEmpty());
            }

            this._needsToRefreshList = false;
        }

        private void listBox_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            var dep = (DependencyObject)e.OriginalSource;
            dep = dep.FindParent<ListBoxItem>();

            if (dep == null)
                return;

            var index = listBox.ItemContainerGenerator.IndexFromContainer(dep);

            e.Handled = true;
            listBox.SelectedIndex = index;

            this.confirmCompletion(false);
        }

        private void ActionButton_OnClick(object sender, RoutedEventArgs e)
        {
            ActionButtonClick?.Invoke(sender, e);
            if (KeepOpenWhenSelecting)
            {
                this._textbox.Focus();
            }

            this.actionButton.ShowOnlyIf(this._controller.ShowActionButton && !ActionButtonText.IsNullOrEmpty());
        }

        public bool HasKeyboardFocus() => popup.IsKeyboardFocusWithin;
    }
}

