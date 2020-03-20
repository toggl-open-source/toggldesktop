using System;
using System.Windows;

namespace TogglDesktop
{
    public partial class AutotrackerNotification
    {
        private readonly Action _startButtonClick;

        public AutotrackerNotification(Action close, Action showParentWindow, Action startButtonClick)
            : base(close, showParentWindow)
        {
            _startButtonClick = startButtonClick;

            InitializeComponent();
        }

        private void onStartButtonClick(object sender, RoutedEventArgs e)
        {
            _startButtonClick();
        }
    }
}