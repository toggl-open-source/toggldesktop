
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

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
                    "#4dc3ff", "#bc85e6", "#df7baa", "#f68d38", "#b27636",
                    "#8ab734", "#14a88e", "#268bb5", "#6668b4", "#a4506c",
                    "#67412c", "#3c6526", "#094558", "#bc2d07", "#999999"
                };
            }
        }
    }

    public partial class ProjectColorPicker
    {
        private readonly Random random = new Random();

        private string selectedColor;
        private string[] colors;

        public ProjectColorPicker()
        {
            this.InitializeComponent();

            Toggl.OnDisplayProjectColors += this.onDisplayProjectColors;
        }

        private void onDisplayProjectColors(string[] strings, ulong count)
        {
            if (this.TryBeginInvoke(this.onDisplayProjectColors, strings, count))
                return;

            this.colors = strings;
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
            var open = !this.popup.IsOpen;

            if (open)
            {
                this.open();
            }
            else
            {
                this.close(true);
            }
        }

        private void close(bool focusButton = false)
        {
            this.popup.IsOpen = false;
            if (focusButton)
            {
                this.button.Focus();   
            }
        }

        private void open()
        {
            if (!this.ensureProjectColors())
                return;

            this.popup.IsOpen = true;

            var i = 0;
            if (this.selectedColor != null)
            {
                var argb = Utils.ProjectColorFromString(this.selectedColor);

                i = Array.FindIndex(this.colors,
                    c => Utils.ProjectColorFromString(c) == argb);
                if (i < 0)
                    i = 0;
            }
            this.list.SelectedIndex = i;
            ((Control)this.list.ItemContainerGenerator.ContainerFromIndex(i)).Focus();
        }

        private bool ensureProjectColors()
        {
            if (this.colors != null)
                return true;

            Toggl.GetProjectColors();

            if (this.colors != null)
                return true;

            Toggl.Debug("Error: Did not receive project colours when required.");
            return false;
        }

        private void onColorSelect(object sender, RoutedEventArgs e)
        {
            this.SelectedColor = (string)this.list.SelectedItem;

            this.close(true);
        }

        private void onPopupPreviewKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Escape:
                {
                    this.close(true);
                    e.Handled = true;
                    break;
                }
                case Key.Enter:
                {
                    this.SelectedColor = (string)this.list.SelectedItem;
                    this.close(true);
                    e.Handled = true;
                    break;
                }
            }
        }


        private void onPopupKeyboardFocusWithinChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (!this.popup.IsKeyboardFocusWithin && !this.button.IsKeyboardFocusWithin)
            {
                this.close();
            }
        }

        public void SelectRandom()
        {
            if (!this.ensureProjectColors())
                return;

            this.SelectedColor = this.colors.RandomElement(this.random);
        }
    }
}
