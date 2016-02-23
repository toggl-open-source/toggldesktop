using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;

namespace TogglDesktop
{
    static class TimeEntryCellDragImposter
    {
        private static Window window;
        private static TimeEntryCell imposter;
        private static Vector mouseOffset;

        public static void Start(TimeEntryCell cell)
        {
            ensureInitialised();
            imposter.Imitate(cell);
            mouseOffset = new Point(0, 0) - Mouse.GetPosition(cell);
            var cellPosition = cell.PointToScreen(new Point(0, 0));
            window.Left = cellPosition.X;
            window.Top = cellPosition.Y;
            window.Width = cell.ActualWidth;
            window.Height = cell.ActualHeight;
            window.Visibility = Visibility.Visible;
        }

        public static void Update()
        {
            // WPF mouse position is broken during drag. Using winapi instead.
            var p = default(Win32.Win32Point);
            Win32.GetCursorPos(ref p);

            window.Left = p.X + mouseOffset.X;
            window.Top = p.Y + mouseOffset.Y;
        }

        private static void ensureInitialised()
        {
            if (imposter != null)
                return;

            window = new Window
            {
                Topmost = true,
                WindowStyle = WindowStyle.None,
                AllowsTransparency = true,
                IsHitTestVisible = false,
                Opacity = 0.5,
                AllowDrop = false,
            };
            imposter = new TimeEntryCell
            {
                IsDummy = true,
                IsHitTestVisible = false,
                AllowDrop = false,
            };
            window.Content = imposter;
            window.SourceInitialized += (sender, args) =>
            {
                var hwnd = new WindowInteropHelper(window).Handle;
                Win32.SetWindowExTransparent(hwnd);
            };
        }

        public static void Stop()
        {
            window.Visibility = Visibility.Collapsed;
        }
    }
}