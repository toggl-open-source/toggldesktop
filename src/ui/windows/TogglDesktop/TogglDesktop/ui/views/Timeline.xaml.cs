using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using DynamicData.Binding;
using ReactiveUI;
using TogglDesktop.Resources;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for Timeline.xaml
    /// </summary>
    public partial class Timeline : UserControl
    {
        private CompositeDisposable _disposable;
        public TimelineViewModel ViewModel
        {
            get => DataContext as TimelineViewModel;
            set => DataContext = value;
        }

        public Timeline()
        {
            InitializeComponent();
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            if (e.Property.Name == nameof(DataContext))
            {
                _disposable?.Dispose();
                _disposable = new CompositeDisposable();

                ViewModel?.WhenAnyValue(x => x.SelectedScaleMode).Buffer(2, 1)
                    .ObserveOn(RxApp.MainThreadScheduler)
                    .Select(b => (double)TimelineConstants.ScaleModes[b[1]] / TimelineConstants.ScaleModes[b[0]])
                    .Subscribe(ratio => SetMainViewScrollOffset(MainViewScroll.VerticalOffset * ratio))
                    .DisposeWith(_disposable);
                ViewModel?.WhenValueChanged(x => x.IsTodaySelected)
                    .Where(x => x)
                    .ObserveOn(RxApp.MainThreadScheduler)
                    .Subscribe(_ => MainViewScroll.ScrollToVerticalOffset(ViewModel.CurrentTimeOffset - MainViewScroll.ActualHeight / 2))
                    .DisposeWith(_disposable);
                ViewModel?.WhenAnyValue(x => x.FirstTimeEntryOffset)
                    .ObserveOn(RxApp.MainThreadScheduler)
                    .Where(_ => !ViewModel.IsTodaySelected)
                    .Subscribe(SetMainViewScrollOffset)
                    .DisposeWith(_disposable);
            }
        }

        public void SetMainViewScrollOffset(double offset)
        {
            if (this.TryBeginInvoke(SetMainViewScrollOffset, offset))
                return;

            MainViewScroll.ScrollToVerticalOffset(offset);
        }

        private void HandleScrollViewerMouseWheel(object sender, MouseWheelEventArgs e)
        {
            MainViewScroll.ScrollToVerticalOffset(MainViewScroll.VerticalOffset - e.Delta);
            e.Handled = true;
        }

        private void OnActivityMouseEnter(object sender, MouseEventArgs e)
        {
            if (sender is FrameworkElement uiElement && uiElement.DataContext is TimelineViewModel.ActivityBlock curBlock)
            {
                ViewModel.SelectedActivityBlock = curBlock;
                ActivityBlockPopup.PlacementTarget = uiElement;
                ActivityBlockPopup.VerticalOffset = uiElement.Height/2;
            }
        }

        private void OnMainViewScrollLoaded(object sender, RoutedEventArgs e)
        {
            if (ViewModel?.IsTodaySelected == true)
                MainViewScroll.ScrollToVerticalOffset(ViewModel.CurrentTimeOffset - MainViewScroll.ActualHeight / 2);
        }

        private void OnTimeEntryBlockMouseEnter(object sender, MouseEventArgs e)
        {
            if (sender is FrameworkElement uiElement && uiElement.DataContext is TimeEntryBlock curBlock)
            {
                ViewModel.SelectedTimeEntryBlock = curBlock;
                TimeEntryPopup.PlacementTarget = uiElement;
                TimeEntryPopup.IsOpen = true;
                var visibleTopOffset = MainViewScroll.VerticalOffset+10;
                var visibleBottomOffset = MainViewScroll.VerticalOffset + MainViewScroll.ActualHeight-10;
                var offset = curBlock.VerticalOffset + uiElement.ActualHeight / 2;
                TimeEntryPopup.VerticalOffset = Math.Min(Math.Max(visibleTopOffset, offset), visibleBottomOffset) -
                                                curBlock.VerticalOffset;
            }
        }

        private void OnTimeEntyrBlockMouseLeave(object sender, MouseEventArgs e)
        {
            if (sender is FrameworkElement uiElement && uiElement.DataContext is TimeEntryBlock curBlock)
            {
                TimeEntryPopup.IsOpen = false;
            }
        }

        private double? _dragStartedPoint;
        private string _timeEntryId;
        private void OnTimeEntryCanvasMouseDown(object sender, MouseButtonEventArgs e)
        {
            _dragStartedPoint = e.GetPosition(TimeEntryBlocks).Y;
            _timeEntryId = ViewModel.AddNewTimeEntry(_dragStartedPoint.Value);
        }

        private void OnTimeEntryCanvasMouseMove(object sender, MouseEventArgs e)
        {
            if (_dragStartedPoint != null && _timeEntryId != null && e.LeftButton == MouseButtonState.Pressed)
            {
                var verticalChange = Math.Abs(e.GetPosition(TimeEntryBlocks).Y - _dragStartedPoint.Value);
                ViewModel.TimeEntryBlocks[_timeEntryId].VerticalOffset =
                    Math.Min(_dragStartedPoint.Value, e.GetPosition(TimeEntryBlocks).Y);
                ViewModel.TimeEntryBlocks[_timeEntryId].Height = verticalChange;
            }
        }

        private void OnTimeEntryCanvasDragEnded(object sender, DragEventArgs e)
        {
            if (_timeEntryId != null)
            {
                ViewModel.TimeEntryBlocks[_timeEntryId].ChangeStartTime();
                ViewModel.TimeEntryBlocks[_timeEntryId].ChangeEndTime();
            }
            _dragStartedPoint = null;
            _timeEntryId = null;
        }
    }
}
