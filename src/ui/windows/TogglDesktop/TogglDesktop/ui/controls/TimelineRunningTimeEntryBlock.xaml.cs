using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for TimelineRunningTimeEntryBlock.xaml
    /// </summary>
    public partial class TimelineRunningTimeEntryBlock : UserControl
    {
        public TimeEntryBlock ViewModel
        {
            get => DataContext as TimeEntryBlock;
            set => DataContext = value;
        }

        public TimelineRunningTimeEntryBlock()
        {
            InitializeComponent();
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

        private void OnThumbDragStarted(object sender, DragStartedEventArgs e)
        {
            ViewModel.IsDragged = true;
        }

        private TimelineTimeEntryBlockPopup _popup;
        protected override void OnMouseEnter(MouseEventArgs e)
        {
            _popup ??= new TimelineTimeEntryBlockPopup();
            _popup.DataContext = ViewModel;
            _popup.Popup.PlacementTarget = this;
            _popup.Popup.IsOpen = true;
        }

        protected override void OnMouseLeave(MouseEventArgs e)
        {
            _popup.Popup.IsOpen = false;
        }
    }
}
