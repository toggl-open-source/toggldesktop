using DynamicData.Binding;
using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;
using TogglDesktop.Diagnostics;
using TogglDesktop.ui.ViewModels;

namespace TogglDesktop
{
    public partial class Timer
    {
        public TimerViewModel ViewModel
        {
            get => (TimerViewModel)DataContext;
            set => DataContext = value;
        }

        public event EventHandler FocusTimeEntryList;
        public event EventHandler<string> DescriptionTextBoxTextChanged;

        public Timer()
        {
            this.InitializeComponent();
            ViewModel = new TimerViewModel(false);
            ViewModel.WhenValueChanged(x => x.IsRunning).Where(isRunning => !isRunning).Subscribe(_ => FocusFirstInput());
            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
        }

        #region toggl events

        private void onMiniTimerAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onMiniTimerAutocomplete, list))
                return;

            using (Performance.Measure("timer building auto complete controller, {0} items", list.Count))
            {
                this.descriptionAutoComplete.SetController(AutoCompleteControllersFactory.ForTimer(list));
            }
        }

        #endregion

        #region ui events

        private void onProjectLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            TryOpenEditViewIfRunning(e, Toggl.Project);
        }

        private void onDescriptionLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            TryOpenEditViewIfRunning(e, Toggl.Description);
        }

        private void onTimeLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            TryOpenEditViewIfRunning(e, Toggl.Duration);
        }

        private void onGridMouseDown(object sender, MouseButtonEventArgs e)
        {
            TryOpenEditViewIfRunning(e, "");
        }

        private void TryOpenEditViewIfRunning(MouseButtonEventArgs e, string focusedField)
        {
            ViewModel.tryOpenEditViewIfRunning(focusedField);
            e.Handled = true;
        }

        private void descriptionAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            ViewModel.DescriptionAutoCompleteConfirm(e);
            descriptionTextBox.CaretIndex = descriptionTextBox.Text.Length;
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            if (e.Key == Key.Down && Keyboard.Modifiers == ModifierKeys.Shift)
            {
                if (this.FocusTimeEntryList != null)
                    this.FocusTimeEntryList(this, e);
                e.Handled = true;
            }

            base.OnPreviewKeyDown(e);
        }

        protected override void OnGotKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            FocusFirstInput();
        }

        private void FocusFirstInput()
        {
            if (this.TryBeginInvoke(FocusFirstInput))
                return;

            if (!this.IsKeyboardFocused)
                return;

            if (this.manualPanel.IsVisible)
            {
                this.manualAddButton.Focus();
            }
            else if (this.descriptionTextBox.IsVisible)
            {
                this.descriptionTextBox.Focus();
            }
        }

        #endregion

        public void SetManualMode(bool manualMode)
        {
            this.manualPanel.ShowOnlyIf(manualMode);
            this.timerPanel.ShowOnlyIf(!manualMode);
        }

        private void onDescriptionTextBoxTextChanged(object sender, TextChangedEventArgs e)
        {
            DescriptionTextBoxTextChanged?.Invoke(sender, this.descriptionTextBox.Text);
        }

        public event Action StartStopButtonClicked;
        private void startStopButton_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            StartStopButtonClicked?.Invoke();
        }
    }
}