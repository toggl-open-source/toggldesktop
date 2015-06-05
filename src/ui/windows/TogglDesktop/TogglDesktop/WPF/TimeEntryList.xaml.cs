using System;
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
    }
}
