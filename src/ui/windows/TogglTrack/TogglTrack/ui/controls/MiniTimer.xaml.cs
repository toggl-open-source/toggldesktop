using DynamicData.Binding;
using System;
using System.Collections.Generic;
using System.Windows.Input;
using TogglTrack.AutoCompletion;
using TogglTrack.AutoCompletion.Items;
using TogglTrack.Diagnostics;
using TogglTrack.ui.ViewModels;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;

namespace TogglTrack
{
    public partial class MiniTimer
    {
        public TimerViewModel ViewModel
        {
            get => (TimerViewModel)DataContext;
            set => DataContext = value;
        }

        public MiniTimer()
        {
            this.InitializeComponent();
            ViewModel = new TimerViewModel(true);
            ViewModel.WhenValueChanged(x => x.IsRunning).Subscribe(x => { if (!x) FocusFirstInput(); });

            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
        }

        public event MouseButtonEventHandler MouseCaptured;

       

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
            if (e.ClickCount == 2)
            {
                ViewModel.tryOpenEditViewIfRunning(focusedField);
                e.Handled = true;
            }
        }

        private void descriptionAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            ViewModel.DescriptionAutoCompleteConfirm(e);
            descriptionTextBox.CaretIndex = descriptionTextBox.Text.Length;
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

        private void MiniTimer_OnPreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (!startStopButton.IsMouseOver)
            {
                MouseCaptured?.Invoke(sender, e);
            }
        }
    }
}