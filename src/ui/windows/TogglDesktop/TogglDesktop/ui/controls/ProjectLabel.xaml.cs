using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TogglDesktop
{
    public partial class ProjectLabel : UserControl
    {
        public ProjectLabel()
        {
            InitializeComponent();
        }

        public bool ShowAddProjectCaption { get; set; }

        public static readonly DependencyProperty ColorProperty = DependencyProperty.Register(
            "Color", typeof(Brush), typeof(ProjectLabel), new PropertyMetadata(default(Brush)));

        public Brush Color
        {
            get { return (Brush) GetValue(ColorProperty); }
            set { SetValue(ColorProperty, value); }
        }

        public static readonly DependencyProperty ProjectNameProperty = DependencyProperty.Register(
            "ProjectName", typeof(string), typeof(ProjectLabel), new PropertyMetadata(default(string), OnProjectNameChanged));

        public string ProjectName
        {
            get { return (string) GetValue(ProjectNameProperty); }
            set { SetValue(ProjectNameProperty, value); }
        }

        public static readonly DependencyProperty TaskNameProperty = DependencyProperty.Register(
            "TaskName", typeof(string), typeof(ProjectLabel), new PropertyMetadata(default(string)));

        public string TaskName
        {
            get { return (string) GetValue(TaskNameProperty); }
            set { SetValue(TaskNameProperty, value); }
        }

        public static readonly DependencyProperty ClientNameProperty = DependencyProperty.Register(
            "ClientName", typeof(string), typeof(ProjectLabel), new PropertyMetadata(default(string)));

        public string ClientName
        {
            get { return (string) GetValue(ClientNameProperty); }
            set { SetValue(ClientNameProperty, value); }
        }

        private static void OnProjectNameChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            var projectLabel = obj as ProjectLabel;
            var showAddProjectCaption = projectLabel.ShowAddProjectCaption && projectLabel.IsEmpty;
            projectLabel.addProjectCaption.ShowOnlyIf(showAddProjectCaption);
        }

        public bool IsEmpty => string.IsNullOrEmpty(ProjectName);

        public void Clear()
        {
            ProjectName = string.Empty;
            TaskName = string.Empty;
            ClientName = string.Empty;
            Color = default;
        }

        public void SetProject(Toggl.TogglTimeEntryView item)
        {
            ProjectName = item.ProjectLabel;
            TaskName = item.TaskLabel;
            ClientName = item.ClientLabel;
            Color = Utils.ProjectColorBrushFromString(item.Color);
        }
    }
}