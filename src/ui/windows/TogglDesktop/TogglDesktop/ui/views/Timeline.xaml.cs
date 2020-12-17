using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using DynamicData.Binding;
using ReactiveUI;
using TogglDesktop.Behaviors;
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
                ViewModel?.FistTimeEntryOffsetForSelectedDate
                    .ObserveOn(RxApp.MainThreadScheduler)
                    .Where(offset => !ViewModel.IsTodaySelected && offset.HasValue)
                    .Subscribe(offset => SetMainViewScrollOffset(offset.Value))
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
            if (sender is FrameworkElement uiElement)
            {
                TimeEntryPopupContainer.OpenPopupWithRightPlacement(uiElement, MainViewScroll);
            }
        }

        private void OnTimeEntyrBlockMouseLeave(object sender, MouseEventArgs e)
        {
            TimeEntryPopupContainer.ClosePopup();
        }

        private double? _dragStartedPoint;
        private string _timeEntryId;
        private void OnTimeEntryCanvasMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                Mouse.Capture(sender as UIElement);
                var point = e.GetPosition(TimeEntryBlocks);
                _dragStartedPoint = point.Y;
                _timeEntryId = TimelineViewModel.AddNewTimeEntry(_dragStartedPoint.Value, 0, ViewModel.SelectedScaleMode, ViewModel.SelectedDate);
            }
        }

        private void OnTimeEntryCanvasMouseMove(object sender, MouseEventArgs e)
        {
            if (_dragStartedPoint != null && _timeEntryId != null && e.LeftButton == MouseButtonState.Pressed)
            {
                ViewModel.TimeEntryBlocks[_timeEntryId].IsDragged = true;
                var point = e.GetPosition(TimeEntryBlocks);
                var verticalChange = Math.Abs(point.Y - _dragStartedPoint.Value);
                ViewModel.TimeEntryBlocks[_timeEntryId].VerticalOffset =
                    Math.Min(_dragStartedPoint.Value, point.Y);
                ViewModel.TimeEntryBlocks[_timeEntryId].Height = verticalChange;
                if (!TimeEntryPopupContainer.Popup.IsOpen)
                    TimeEntryPopupContainer.OpenPopup(PlacementMode.Relative, TimeEntryBlocks, ViewModel.TimeEntryBlocks[_timeEntryId],
                        point.X + 15, point.Y);
                else
                    TimeEntryPopupContainer.PlacePopup(PlacementMode.Relative, TimeEntryBlocks, point.X + 15, point.Y);
            }
        }

        private void OnTimeEntryCanvasMouseUp(object sender, MouseButtonEventArgs mouseButtonEventArgs)
        {
            if (_timeEntryId != null && _dragStartedPoint != null)
            {
                if (Math.Abs(mouseButtonEventArgs.GetPosition(TimeEntryBlocks).Y - _dragStartedPoint.Value) <= 2)
                {
                    ViewModel.TimeEntryBlocks[_timeEntryId].Height = TimelineConstants.DefaultTimeEntryLengthInHours*TimelineConstants.ScaleModes[ViewModel.SelectedScaleMode];
                }
                ViewModel.TimeEntryBlocks[_timeEntryId].ChangeStartEndTime();
                ViewModel.TimeEntryBlocks[_timeEntryId].IsDragged = false;
                Toggl.Edit(_timeEntryId, false, Toggl.Description);
            }
            TimeEntryPopupContainer.ClosePopup();
            _dragStartedPoint = null;
            _timeEntryId = null;
            Mouse.Capture(null);
        }

        private void OnTimeEntryContextMenuOpen(object sender, ContextMenuEventArgs contextMenuEventArgs)
        {
            if (sender is FrameworkElement uiElement)
            {
                ViewModel.ActiveTimeEntryBlock = uiElement.DataContext as TimeEntryBlock;
            }
        }
    }
}
