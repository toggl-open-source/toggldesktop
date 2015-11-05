
using System.Windows;

namespace TogglDesktop
{
    public sealed class ProjectColorPickerSample
    {
        public string[] Colors
        {
            get
            {
                return new[]
                {
                    "#999999", "#ff00ff", "#00ff00", "#0048BA", "#E52B50", "#841B2D",
                    "#999999", "#ff00ff", "#00ff00", "#0048BA", "#E52B50", "#841B2D"
                };
            }
        }
    }

    public partial class ProjectColorPicker
    {
        private string selectedColor;

        public ProjectColorPicker()
        {
            this.InitializeComponent();

            Toggl.OnDisplayProjectColors += this.onDisplayProjectColors;

            this.onDisplayProjectColors(new[]
                {
                    "#999999", "#ff00ff", "#00ff00", "#0048BA", "#E52B50", "#841B2D",
                    "#999999", "#ff00ff", "#00ff00", "#0048BA", "#E52B50", "#841B2D"
                }, 0);
        }

        private void onDisplayProjectColors(string[] strings, ulong count)
        {
            if (this.TryBeginInvoke(this.onDisplayProjectColors, strings, count))
                return;

            this.list.ItemsSource = strings;
        }

        public string SelectedColor
        {
            get { return this.selectedColor; }
            set
            {
                this.selectedColor = value;
                this.updateColor();
            }
        }

        private void updateColor()
        {
            this.colorCircle.Background =
                Utils.ProjectColorBrushFromString(this.selectedColor);
        }


        private void onButtonClick(object sender, RoutedEventArgs e)
        {
            this.popup.IsOpen = !this.popup.IsOpen;
        }
    }
}
