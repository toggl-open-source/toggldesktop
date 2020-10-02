using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using ReactiveUI;
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
                    .Select(b => (double)ViewModel.ScaleModes[b[1]] / ViewModel.ScaleModes[b[0]])
                    .Subscribe(ratio => MainViewScroll.ScrollToVerticalOffset(MainViewScroll.VerticalOffset * ratio))
                    .DisposeWith(_disposable);
            }
        }

        private void RecordActivityInfoBoxOnMouseEnter(object sender, MouseEventArgs e)
        {
            RecordActivityInfoPopup.IsOpen = true;
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

        private void OnTimeEntryBlockMouseEnter(object sender, MouseEventArgs e)
        {
            if (sender is FrameworkElement uiElement && uiElement.DataContext is TimeEntryBlock curBlock)
            {
                ViewModel.SelectedTimeEntryBlock = curBlock;
                TimeEntryPopup.PlacementTarget = uiElement;
                TimeEntryPopup.VerticalOffset = uiElement.ActualHeight / 2;
                TimeEntryPopup.IsOpen = true;
            }
        }

        private void OnTimeEntyrBlockMouseLeave(object sender, MouseEventArgs e)
        {
            if (sender is FrameworkElement uiElement && uiElement.DataContext is TimeEntryBlock curBlock)
            {
                TimeEntryPopup.IsOpen = false;
            }
        }
    }
}
