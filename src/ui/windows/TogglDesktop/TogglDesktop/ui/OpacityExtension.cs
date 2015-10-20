using System;
using System.Windows.Markup;
using System.Windows.Media;

namespace TogglDesktop
{
    public class OpacityExtension : MarkupExtension
    {
        public Color Color { get; set; }
        public byte Opacity { get; set; }

        public OpacityExtension()
        {
        }

        public OpacityExtension(Color color, double opacity)
        {
            this.Color = color;
            this.Opacity = (byte)(255 * opacity);
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return Color.FromArgb(this.Opacity, this.Color.R, this.Color.G, this.Color.B);
        }
    }
}