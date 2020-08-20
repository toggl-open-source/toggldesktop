using TogglDesktop.Theming;

namespace TogglDesktop
{
    public static class ColorSchemeExtensions
    {
        public static Toggl.TogglAdaptiveColor ToAdaptiveShapeColor(this ColorScheme scheme)
        {
            return scheme switch
            {
                ColorScheme.Dark => Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnDarkBackground,
                ColorScheme.Dark_Legacy => Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnDarkBackground,
                _ => Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground
            };
        }

        public static Toggl.TogglAdaptiveColor ToAdaptiveTextColor(this ColorScheme scheme)
        {
            return scheme switch
            {
                ColorScheme.Dark => Toggl.TogglAdaptiveColor.AdaptiveColorTextOnDarkBackground,
                ColorScheme.Dark_Legacy => Toggl.TogglAdaptiveColor.AdaptiveColorTextOnDarkBackground,
                _ => Toggl.TogglAdaptiveColor.AdaptiveColorTextOnLightBackground
            };
        }
    }
}