using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for TimeEntryList.xaml
    /// </summary>
    public partial class TimeEntryList
    {
        private Func<bool> canFocus;

        public TimeEntryList()
        {
            InitializeComponent();
        }

        public UIElementCollection Children
        {
            get { return panel.Children; }
        }

        private void onMouseEnter(object sender, MouseEventArgs e)
        {
            // FIXME: this condition seems to be always true
            if (canFocus())
            {
                scrollViewer.Focus();
            }
        }

        public void SetFocusCondition(Func<bool> canFocus)
        {
            this.canFocus = canFocus;
        }

        public void HighlightCell(TimeEntryCell cell)
        {
            if (cell != null)
            {
                var y = cell.TransformToAncestor(this.panel).Transform(new Point(0, 0)).Y + cell.ActualHeight;
                this.highlightRectangleTop.Height = y - 53;
                this.highlightRectangleBottom.Height = this.panel.ActualHeight - y;
                this.highlightRectangleTop.Visibility = Visibility.Visible;
                this.highlightRectangleBottom.Visibility = Visibility.Visible;
            }
            else
            {
                this.highlightRectangleTop.Height = this.panel.ActualHeight;
                this.highlightRectangleBottom.Height = 0;
                this.highlightRectangleTop.Visibility = Visibility.Visible;
                this.highlightRectangleBottom.Visibility = Visibility.Collapsed;
            }
        }

        public void DisableHighlight()
        {
            this.highlightRectangleTop.Visibility = Visibility.Collapsed;
            this.highlightRectangleBottom.Visibility = Visibility.Collapsed;
            this.highlightRectangleTop.Height = 0;
            this.highlightRectangleBottom.Height = 0;
        }
    }
}
