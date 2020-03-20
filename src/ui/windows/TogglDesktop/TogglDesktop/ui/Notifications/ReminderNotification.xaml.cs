using System;
using System.Windows;

namespace TogglDesktop
{
    public partial class ReminderNotification
    {
        private readonly Action _startButtonClick;

        public ReminderNotification(Action close, Action showParentWindow, Action startButtonClick)
            : base(close, showParentWindow)
        {
            _startButtonClick = startButtonClick;
            
            InitializeComponent();
        }

        private void StartButton_OnClick(object sender, RoutedEventArgs e)
        {
            _startButtonClick();
        }
    }
}