using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Newtonsoft.Json.Bson;

namespace TogglDesktop.WPF
{
    public partial class TimerEditViewController
    {
        private string runningGUID;

        public TimerEditViewController()
        {
            this.InitializeComponent();

            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;

            this.setUIToStoppedState();
        }

        private bool isRunning { get { return this.startStopButton.IsChecked ?? false; } }

        #region helper methods

        private bool invoke(Action action)
        {
            if (this.Dispatcher.CheckAccess())
                return false;
            this.Dispatcher.Invoke(action);
            return true;
        }

        #endregion

        #region toggl events

        private void onStoppedTimerState()
        {
            if (this.invoke(this.onStoppedTimerState))
                return;

            this.runningGUID = null;
            this.setUIToStoppedState();
        }

        private void onRunningTimerState(Toggl.TimeEntry te)
        {
            if (this.invoke(() => this.onRunningTimerState(te)))
                return;

            this.runningGUID = te.GUID;
            this.setUIToRunningState(te);
        }

        private void onMiniTimerAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            if (this.invoke(() => this.onMiniTimerAutocomplete(list)))
                return;

        }

        #endregion

        #region ui events

        private void startStopButtonOnClick(object sender, RoutedEventArgs e)
        {
            if (this.isRunning)
            {
                this.start();
            }
            else
            {
                this.stop();
            }
        }

        private void onProjectLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (this.isRunning)
            {
                Toggl.Edit(this.runningGUID, false, Toggl.Project);
            }
        }

        private void onDescriptionLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (this.isRunning)
            {
                Toggl.Edit(this.runningGUID, false, Toggl.Description);
            }
        }

        private void onTimeLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (this.isRunning)
            {
                Toggl.Edit(this.runningGUID, false, Toggl.Duration);
            }
        }
        #endregion

        #region controlling

        private void start()
        {
            var description = this.descriptionTextBox.Text;
            var duration = this.durationTextBox.Text;

            var success = Toggl.Start(description, duration, 0, 0, "") != null;

            if (success)
            {
                //TODO: reset any stored data from auto completion if needed
            }
        }

        private void stop()
        {
            Toggl.Stop();
        }

        #endregion 

        #region updating ui

        private void setUIToRunningState(Toggl.TimeEntry item)
        {
            this.resetUIState(true);

            this.descriptionLabel.Text = item.Description == "" ? "(no description)" : item.Description;
            this.projectLabel.Text = item.ClientLabel != "" ? "• " + item.ProjectLabel : item.ProjectLabel;
            setOptionalTextBlockText(this.clientLabel, item.ClientLabel);
            setOptionalTextBlockText(this.taskLabel, item.TaskLabel);

            this.projectLabel.Foreground = getProjectColorBrush(ref item);

            showOnlyIf(this.billabeIcon, item.Billable);
            showOnlyIf(this.tagsIcon, !string.IsNullOrEmpty(item.Tags));

            this.projectGridRow.Height = string.IsNullOrEmpty(item.ProjectLabel)
                ? new GridLength(0)
                : new GridLength(1, GridUnitType.Star);

            this.invalidate();
        }

        private void setUIToStoppedState()
        {
            this.resetUIState(false);

            this.durationLabel.Text = "00:00:00";

            this.invalidate();
        }

        private void resetUIState(bool running)
        {
            this.startStopButton.IsChecked = running;
            this.descriptionTextBox.Text = "";
            this.durationTextBox.Text = "";
            showOnlyIf(this.descriptionTextBox, !running);
            showOnlyIf(this.durationTextBox, !running);
            showOnlyIf(this.descriptionProjectGrid, running);
            showOnlyIf(this.iconPanel, running);
        }

        #region display helpers

        private void invalidate()
        {
            //TODO: is this needed?
        }


        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            showOnlyIf(textBlock, !string.IsNullOrEmpty(text));
        }

        private static void showOnlyIf(UIElement element, bool condition)
        {
            element.Visibility = condition ? Visibility.Visible : Visibility.Collapsed;
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TimeEntry item)
        {
            var colourString = string.IsNullOrEmpty(item.Color) ? "#999999" : item.Color;
            var color = (Color)(ColorConverter.ConvertFromString(colourString) ?? Color.FromRgb(153, 153, 153));
            return new SolidColorBrush(color);
        }

        #endregion

        #endregion

    }
}
