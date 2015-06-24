using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;

namespace TogglDesktop.WPF
{
    sealed class AutoCompletionPopupSample
    {
        public FrameworkElement Target { get { return null; } }
        public ExtendedTextBox TextBox { get { return null; } }
        public string EmptyText { get { return "Hello. Yes, this is list."; } }
    }

    partial class AutoCompletionPopup
    {
        public event EventHandler<AutoCompleteItem> ConfirmCompletion;
        public event EventHandler<string> ConfirmWithoutCompletion;

        public event EventHandler IsOpenChanged;

        private ExtendedTextBox textbox;

        private bool needsToRefreshList;

        private AutoCompleteController controller;

        public AutoCompletionPopup()
        {
            this.DataContext = this;
            this.InitializeComponent();

            this.popup.Opened += (s, e) => this.tryInvoke(this.IsOpenChanged);
            this.popup.Closed += (s, e) => this.tryInvoke(this.IsOpenChanged);

            this.IsEnabledChanged += onIsEnabledChanged;
        }

        private void onIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs args)
        {
            if (this.IsEnabled)
                return;

            this.close();
        }

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

        #region dependency properties

        public static readonly DependencyProperty TargetProperty = DependencyProperty
            .Register("Target", typeof(FrameworkElement), typeof(AutoCompletionPopup));
        public FrameworkElement Target
        {
            get { return (FrameworkElement)this.GetValue(TargetProperty); }
            set { this.SetValue(TargetProperty, value); }
        }

        public static readonly DependencyProperty TextBoxProperty = DependencyProperty
            .Register("TextBox", typeof(ExtendedTextBox), typeof(AutoCompletionPopup),
            new FrameworkPropertyMetadata
            {
                PropertyChangedCallback = (o, args) => ((AutoCompletionPopup)o).initialise()
            });
        public ExtendedTextBox TextBox
        {
            get { return (ExtendedTextBox)this.GetValue(TextBoxProperty); }
            set { this.SetValue(TextBoxProperty, value); }
        }

        public static readonly DependencyProperty EmptyTextProperty = DependencyProperty
            .Register("EmptyText", typeof(string), typeof(AutoCompletionPopup), new FrameworkPropertyMetadata("Nothing found."));

        public string EmptyText
        {
            get { return (string)this.GetValue(EmptyTextProperty); }
            set { this.SetValue(EmptyTextProperty, value); }
        }

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
        }

        #endregion

        public void SetController(AutoCompleteController controller)
        {
            this.controller = controller;
            this.needsToRefreshList = true;
        }

        private void textboxOnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.IsEnabled || e.Handled)
                return;

            switch (e.Key)
            {
                case Key.Down:
                    {
                        if (!this.popup.IsOpen)
                            this.open();
                        this.controller.SelectNext();
                        e.Handled = true;
                        return;
                    }
                case Key.Up:
                    {
                        if (this.popup.IsOpen)
                            this.controller.SelectPrevious();
                        e.Handled = true;
                        return;
                    }
                case Key.Escape:
                    {
                        this.close();
                        e.Handled = true;
                        return;
                    }
                case Key.Enter:
                case Key.Tab:
                    {
                        if (this.popup.IsOpen)
                        {
                            this.confirmCompletion();
                            e.Handled = true;
                        }
                        return;
                    }
            }
        }

        private void confirmCompletion()
        {
            var item = this.controller.SelectedItem;

            this.select(item);
        }

        private void select(AutoCompleteItem item)
        {
            this.popup.IsOpen = false;

            if (item == null)
            {
                if (this.ConfirmWithoutCompletion != null)
                    this.ConfirmWithoutCompletion(this, this.textbox.Text);
                return;
            }

            this.textbox.SetText(item.Text);
            this.textbox.CaretIndex = this.textbox.Text.Length;
            if (this.ConfirmCompletion != null)
                this.ConfirmCompletion(this, item);
        }

        private void textboxOnTextChanged(object sender, TextChangedEventArgs e)
        {
            if (!this.IsEnabled)
                return;

            if (this.textbox.IsTextChangingProgrammatically)
                return;

            this.open();
        }

        private void close()
        {
            this.popup.IsOpen = false;
        }
        private void open()
        {
            this.ensureList();
            this.controller.Complete(this.textbox.Text);
            this.emptyLabel.Visibility = this.controller.VisibleItems.Count == 0
                ? Visibility.Visible
                : Visibility.Collapsed;

            this.popup.IsOpen = true;
        }

        private void ensureList()
        {
            if (!this.needsToRefreshList)
                return;

            var timer = Stopwatch.StartNew();

            this.dropDownList.Children.Clear();
            this.controller.FillList(this.dropDownList, this.select);

            Console.WriteLine("Filled autocomplete list with {0} items. Took {1} ms.", this.dropDownList.Children.Count, timer.ElapsedMilliseconds);

            this.needsToRefreshList = false;
        }

        private void tryInvoke(EventHandler e)
        {
            if (e != null)
                e(this, EventArgs.Empty);
        }
    }
}
