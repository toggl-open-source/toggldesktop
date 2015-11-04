using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop
{
    sealed class TogglChromeDesignTimeConverter : FrameworkElement
    {
        private ContentControl window;
        private TogglChrome togglChrome;

        private string title;
        private bool isToolWindow;

        public string Title
        {
            set {this.title = value; this.refreshData(); }
        }

        public bool IsToolWindow
        {
            set { this.isToolWindow = value; this.refreshData(); }
        }

        private void refreshData()
        {
            if (this.togglChrome == null)
                return;

            set(this.window, "WindowStyle", WindowStyle.None);
            set(this.window, "ResizeMode", ResizeMode.NoResize);

            this.togglChrome.DataContext = new TogglChromeSample(this.title);

            this.togglChrome.IsToolWindow= this.isToolWindow;
        }

        protected override void OnVisualParentChanged(DependencyObject oldParent)
        {
            base.OnVisualParentChanged(oldParent);

            this.setup();
        }

        private void setup()
        {
            if (!DesignerProperties.GetIsInDesignMode(this))
                return;

            var parent = this.Parent as FrameworkElement;
            while (parent != null)
            {
                if (parent.Parent == null)
                {
                    var window = parent as ContentControl;

                    if (window != null)
                    {
                        this.window = window;
                        this.togglChrome = new TogglChrome();
                        var oldContent = window.Content as UIElement;
                        window.Content = this.togglChrome;
                        this.togglChrome.SetContent(oldContent);

                        this.refreshData();

                        return;
                    }
                }
                parent = parent.Parent as FrameworkElement;
            }
            throw new Exception("TogglWindowDesignTimeConverter must be descendend of ContentControl.\n\nTry rebuilding.");
        }


        private static void set(object obj, string property, object value)
        {
            obj.GetType().GetProperty(property).SetValue(obj, value);
        }
    }
}