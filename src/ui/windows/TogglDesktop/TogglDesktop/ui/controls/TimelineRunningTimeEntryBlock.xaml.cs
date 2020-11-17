using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using TogglDesktop.Behaviors;
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

        private readonly TimelineTimeEntryBlockPopup _popupContainer = new TimelineTimeEntryBlockPopup();
        private ScrollViewer _scroll;
        protected override void OnMouseEnter(MouseEventArgs e)
        {
            _scroll ??= this.FindParent<ScrollViewer>();
            _popupContainer.OpenPopup(this, _scroll);
        }

        protected override void OnMouseLeave(MouseEventArgs e)
        {
            _popupContainer.ClosePopup();
        }
    }
}
