using System;
using System.Windows;

namespace TogglDesktop
{
    public partial class PomodoroNotification
    {
        private readonly Action _startNew;
        private readonly Action _continueLatest;

        public PomodoroNotification(Action close, Action showParentWindow, Action startNew, Action continueLatest)
            : base(close, showParentWindow)
        {
            _startNew = startNew;
            _continueLatest = continueLatest;

            InitializeComponent();
        }

        private void onContinueButtonClick(object sender, RoutedEventArgs e)
        {
            _continueLatest();
        }

        private void onStartNewButtonClick(object sender, RoutedEventArgs e)
        {
            _startNew();
        }
    }
}
