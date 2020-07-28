using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.Theming;

namespace TogglDesktop
{
    public partial class ProjectColorPicker
    {
        private readonly Random _random = new Random();

        private string _selectedColor;
        private string[] _colors;

        public ProjectColorPicker()
        {
            this.DataContext = this;
            this.InitializeComponent();

            Toggl.OnDisplayProjectColors += this.onDisplayProjectColors;
            Theme.CurrentColorScheme.Subscribe(_ => RefreshColorsList());
        }

        private void onDisplayProjectColors(string[] strings, ulong count)
        {
            if (this.TryBeginInvoke(this.onDisplayProjectColors, strings, count))
                return;

            this._colors = strings;
            this.list.ItemsSource = strings;
        }

        private void RefreshColorsList()
        {
            var itemsSource = this.list.ItemsSource;
            this.list.ItemsSource = null;
            this.list.ItemsSource = itemsSource;
        }

        public string SelectedColor
        {
            get { return this._selectedColor; }
            set
            {
                this._selectedColor = value;
                this.updateColor();
            }
        }

        private void updateColor()
        {
            this.colorCircle.Fill =
                Utils.AdaptedProjectColorBrushFromString(this._selectedColor);
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
            if (this._selectedColor != null)
            {
                var argb = Utils.ProjectColorFromString(this._selectedColor);

                i = Array.FindIndex(this._colors,
                    c => Utils.ProjectColorFromString(c) == argb);
                if (i < 0)
                    i = 0;
            }

            this.list.SelectedIndex = i;
            ((Control)this.list.ItemContainerGenerator.ContainerFromIndex(i)).Focus();
        }

        private bool ensureProjectColors()
        {
            if (this._colors != null)
                return true;

            Toggl.GetProjectColors();

            if (this._colors != null)
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

            this.SelectedColor = this._colors.RandomElement(this._random);
        }
    }
}
