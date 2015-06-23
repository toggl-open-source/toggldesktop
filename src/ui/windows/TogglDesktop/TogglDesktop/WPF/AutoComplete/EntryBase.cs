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

        public EntryBase(Action selectWithClick)
        {
            this.DataContext = this;
            this.MouseEnter += (sender, args) => this.updateBackgroundColor();
            this.MouseLeave += (sender, args) => this.updateBackgroundColor();
            this.MouseDown += (sender, args) => selectWithClick();
        }

        #region dependency properties

        public static readonly DependencyProperty BackgroundColorProperty = DependencyProperty
            .Register("BackgroundColor", typeof(Color), typeof(EntryBase));

        public Color BackgroundColor
        {
            get { return (Color)this.GetValue(BackgroundColorProperty); }
            set { this.SetValue(BackgroundColorProperty, value); }
        }

        #endregion

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
            this.BackgroundColor = this.IsMouseOver
                ? backgroundColorHover
                : this.selected ? backgroundColorSelected : backgroundColor;
        }
    }
}