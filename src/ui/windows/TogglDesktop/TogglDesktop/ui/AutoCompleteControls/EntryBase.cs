using System;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Media;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.AutoCompleteControls
{
    public class EntryBase : UserControl, ISelectable
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColorHover = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);

        private static readonly SolidColorBrush backgroundBrushSelected = new SolidColorBrush(backgroundColorSelected);
        private static readonly SolidColorBrush backgroundBrushHover = new SolidColorBrush(backgroundColorHover);
        private static readonly SolidColorBrush backgroundBrush = new SolidColorBrush(backgroundColor);

        private bool selected;
        private Action selectWithClick;

        static EntryBase()
        {
            backgroundBrush.Freeze();
            backgroundBrushSelected.Freeze();
            backgroundBrushHover.Freeze();
        }

        public EntryBase(Action selectWithClick = null)
        {
            this.DataContext = this;
            this.Background = backgroundBrush;
            this.setClickAction(selectWithClick);
            this.MouseEnter += (sender, args) => this.updateBackgroundColor();
            this.MouseLeave += (sender, args) => this.updateBackgroundColor();
            this.MouseDown += (sender, args) => this.mouseDown();
            this.PreviewMouseDown += (sender, args) => this.previewMouseDown(sender, args);
        }

        // Prevent mouse click triggerning entry edit under the autocomplete dropdown
        private void previewMouseDown(object sender, MouseButtonEventArgs e)
        {
            e.Handled = true;
            this.mouseDown();
        }

        private void mouseDown()
        {
            if (this.selectWithClick != null)
                this.selectWithClick();
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

        protected virtual void updateBackgroundColor()
        {
            this.Background = this.IsMouseOver
                ? backgroundBrushHover
                : this.selected ? backgroundBrushSelected : backgroundBrush;
        }

        protected void setClickAction(Action selectWithClick)
        {
            this.selectWithClick = selectWithClick;
        }

        protected void prepareForRecycling()
        {
            this.selectWithClick = null;
            this.Background = backgroundBrush;
        }
    }
}