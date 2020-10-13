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
            ViewModel.Height += e.VerticalChange;
        }

        private void OnThumbTopDragDelta(object sender, DragDeltaEventArgs e)
        {
            ViewModel.VerticalOffset += e.VerticalChange;
            ViewModel.Height -= e.VerticalChange;
        }

        private void OnThumbTopDragCompleted(object sender, DragCompletedEventArgs e)
        {
            ViewModel.ChangeStartTime();
        }

        private void OnThumbBottomDragCompleted(object sender, DragCompletedEventArgs e)
        {
            ViewModel.ChangeEndTime();
        }
    }
}
