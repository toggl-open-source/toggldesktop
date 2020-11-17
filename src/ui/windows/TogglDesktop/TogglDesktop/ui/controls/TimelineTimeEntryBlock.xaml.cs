using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for TimelineTimeEntryBlock.xaml
    /// </summary>
    public partial class TimelineTimeEntryBlock : UserControl
    {
        public TimeEntryBlock ViewModel
        {
            get => DataContext as TimeEntryBlock;
            set => DataContext = value;
        }

        public TimelineTimeEntryBlock()
        {
            InitializeComponent();

        }

        private void OnThumbBottomDragDelta(object sender, DragDeltaEventArgs e)
        {
            if (ViewModel.Height + e.VerticalChange > 0)
                ViewModel.Height += e.VerticalChange;
        }

        private void OnThumbTopDragDelta(object sender, DragDeltaEventArgs e)
        {
            if (ViewModel.Height - e.VerticalChange > 0)
            {
                ViewModel.VerticalOffset += e.VerticalChange;
                ViewModel.Height -= e.VerticalChange;
            }
        }

        private void OnThumbTopDragCompleted(object sender, DragCompletedEventArgs e)
        {
            ViewModel.ChangeStartTime();
            ViewModel.IsDragged = false;
        }

        private void OnThumbBottomDragCompleted(object sender, DragCompletedEventArgs e)
        {
            ViewModel.ChangeEndTime();
            ViewModel.IsDragged = false;
        }

        private void OnThumbDragStarted(object sender, DragStartedEventArgs e)
        {
            ViewModel.IsDragged = true;
        }
    }
}
