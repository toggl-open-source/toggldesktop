using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF.AutoComplete
{
    public class EntryBase : UserControl, ISelectable
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColorHover = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);
        private bool selected;

        private static readonly SolidColorBrush backgroundBrushSelected = new SolidColorBrush(backgroundColorSelected);
        private static readonly SolidColorBrush backgroundBrushHover = new SolidColorBrush(backgroundColorHover);
        private static readonly SolidColorBrush backgroundBrush = new SolidColorBrush(backgroundColor);

        static EntryBase()
        {
            backgroundBrush.Freeze();
            backgroundBrushSelected.Freeze();
            backgroundBrushHover.Freeze();
        }

        public EntryBase(Action selectWithClick)
        {
            this.DataContext = this;
            this.Background = backgroundBrush;
            this.MouseEnter += (sender, args) => this.updateBackgroundColor();
            this.MouseLeave += (sender, args) => this.updateBackgroundColor();
            this.MouseDown += (sender, args) => selectWithClick();
        }

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                if (this.selected == value)
                    return;
                this.selected = value;
                this.updateBackgroundColor();
            }
        }

        private void updateBackgroundColor()
        {
            this.Background = this.IsMouseOver
                ? backgroundBrushHover
                : this.selected ? backgroundBrushSelected : backgroundBrush;
        }
    }
}