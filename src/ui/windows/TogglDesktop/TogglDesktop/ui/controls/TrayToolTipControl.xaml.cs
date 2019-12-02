using System.Windows;

namespace TogglDesktop
{
    public partial class TrayToolTipControl
    {
        public TrayToolTipControl()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty DescriptionProperty = DependencyProperty.Register(
            "Description", typeof(string), typeof(TrayToolTipControl), new PropertyMetadata(default(string)));

        public string Description
        {
            get { return (string) GetValue(DescriptionProperty); }
            set { SetValue(DescriptionProperty, value); }
        }

        public static readonly DependencyProperty ProjectColorProperty = DependencyProperty.Register(
            "ProjectColor", typeof(string), typeof(TrayToolTipControl), new PropertyMetadata(default(string)));

        public string ProjectColor
        {
            get { return (string) GetValue(ProjectColorProperty); }
            set { SetValue(ProjectColorProperty, value); }
        }

        public static readonly DependencyProperty TotalTodayProperty = DependencyProperty.Register(
            "TotalToday", typeof(string), typeof(TrayToolTipControl), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty ProjectAndTaskProperty = DependencyProperty.Register(
            "ProjectAndTask", typeof(string), typeof(TrayToolTipControl), new PropertyMetadata(default(string)));

        public string ProjectAndTask
        {
            get { return (string) GetValue(ProjectAndTaskProperty); }
            set { SetValue(ProjectAndTaskProperty, value); }
        }

        public string TotalToday
        {
            get { return (string) GetValue(TotalTodayProperty); }
            set { SetValue(TotalTodayProperty, value); }
        }

        public static readonly DependencyProperty RunningEntryDurationProperty = DependencyProperty.Register(
            "RunningEntryDuration", typeof(string), typeof(TrayToolTipControl), new PropertyMetadata("0 h 00 min"));

        public string RunningEntryDuration
        {
            get { return (string) GetValue(RunningEntryDurationProperty); }
            set { SetValue(RunningEntryDurationProperty, value); }
        }

        public static readonly DependencyProperty IsTrackingProperty = DependencyProperty.Register(
            "IsTracking", typeof(bool), typeof(TrayToolTipControl), new PropertyMetadata(default(bool)));

        public bool IsTracking
        {
            get { return (bool) GetValue(IsTrackingProperty); }
            set { SetValue(IsTrackingProperty, value); }
        }

        public static readonly DependencyProperty ClientProperty = DependencyProperty.Register(
            "Client", typeof(string), typeof(TrayToolTipControl), new PropertyMetadata(default(string)));

        public string Client
        {
            get { return (string) GetValue(ClientProperty); }
            set { SetValue(ClientProperty, value); }
        }
    }
}