using System;
using System.Windows;
using System.Windows.Interop;

namespace TogglDesktop.WPF
{
    public partial class ShadowWindow
    {
        private readonly Window owner;
        private bool visible;

        public ShadowWindow(Window window)
        {
            this.owner = window;
            this.InitializeComponent();

            var interopHelper = new WindowInteropHelper(this);

            Win32.SetWindowExTransparent(interopHelper.Handle);

            window.SizeChanged += (sender, args) => this.updatePosition();
            window.LocationChanged += (sender, args) => this.updatePosition();
            window.IsVisibleChanged += (sender, args) =>
            {
                if (!window.IsVisible)
                    this.Hide();
            };
        }

        protected override void OnActivated(EventArgs e)
        {
            base.OnActivated(e);
            this.owner.Activate();
        }

        private void updatePosition()
        {
            if (this.visible)
            {
                this.Top = this.owner.Top - 10;
                this.Left = this.owner.Left - 10;
                this.Width = this.owner.Width + 20;
                this.Height = this.owner.Height + 25;
            }
            else
            {
                this.Width = 0;
                this.Height = 0;
            }
        }

        public void SetVisibility(bool visible)
        {
            if (this.visible == visible)
                return;

            this.visible = visible;
            this.updatePosition();
        }
    }
}
