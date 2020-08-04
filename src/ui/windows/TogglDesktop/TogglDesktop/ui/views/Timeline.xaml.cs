using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for Timeline.xaml
    /// </summary>
    public partial class Timeline : UserControl
    {
        public TimelineViewModel ViewModel
        {
            get => DataContext as TimelineViewModel;
            set => DataContext = value;
        }

        public Timeline()
        {
            InitializeComponent();
            this.Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, EventArgs args)
        {
            ViewModel = new TimelineViewModel();
        }

        private void RecordActivityInfoBoxOnMouseEnter(object sender, MouseEventArgs e)
        {
            RecordActivityInfoPopup.IsOpen = true;
        }
    }
}
