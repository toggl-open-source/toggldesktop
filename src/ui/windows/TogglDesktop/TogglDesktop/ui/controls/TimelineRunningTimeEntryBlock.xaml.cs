using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using TogglDesktop.ViewModels;

namespace TogglDesktop.ui.controls
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
        }
    }
}
