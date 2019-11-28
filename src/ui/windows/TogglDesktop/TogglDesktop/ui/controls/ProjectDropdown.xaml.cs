using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop
{
    public partial class ProjectDropdown : UserControl
    {
        public ProjectDropdown()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty SelectedProjectProperty = DependencyProperty.Register(
            "SelectedProject", typeof(Toggl.TogglAutocompleteView), typeof(ProjectDropdown),
            new UIPropertyMetadata(default(Toggl.TogglAutocompleteView), OnSelectedProjectChanged));

        private static void OnSelectedProjectChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var projectDropdown = (ProjectDropdown) d;
            var project = (e.NewValue as Toggl.TogglAutocompleteView?).GetValueOrDefault(default);
            projectDropdown.defaultProjectTextBox.SetText(project.ProjectLabel, project.TaskLabel);
            projectDropdown.defaultProjectColorCircle.Background = Utils.ProjectColorBrushFromString(project.ProjectColor);
            projectDropdown.defaultProjectTextBox.CaretIndex = projectDropdown.defaultProjectTextBox.Text.Length;
        }

        public Toggl.TogglAutocompleteView? SelectedProject
        {
            get { return (Toggl.TogglAutocompleteView) GetValue(SelectedProjectProperty); }
            set { SetValue(SelectedProjectProperty, value); }
        }

        public AutoCompleteController AutoCompleteController
        {
            get => defaultProjectAutoComplete.Controller;
            set => defaultProjectAutoComplete.Controller = value;
        }

        private void defaultProjectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.defaultProjectTextBox.Text == "")
            {
                this.SelectedProject = default;
            }
        }

        private void defaultProjectAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            if (e is TimerItem asProjectItem)
            {
                SelectedProject = asProjectItem.Item;
            }
        }

        private void defaultProjectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            SelectedProject = null;
        }
    }
}