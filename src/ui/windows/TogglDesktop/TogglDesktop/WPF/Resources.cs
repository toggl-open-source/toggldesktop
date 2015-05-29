using System;
using System.Windows;
using System.Windows.Markup;

namespace TogglDesktop.WPF
{
    class Resources : MarkupExtension
    {
        private static readonly ResourceDictionary resources;

        static Resources()
        {
            resources = new ResourceDictionary()
            {
                Source = new Uri("pack://application:,,,/TogglDesktop;component/WPF/Resources/Styles.xaml")
            };
        }

        public Resources()
        {

        }

        public String ResourceKey { get; set; }

        public override object ProvideValue(System.IServiceProvider serviceProvider)
        {
            return resources[ResourceKey];
        }
    }
}
