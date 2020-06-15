using Xunit;

namespace TogglDesktop.Tests
{
    public class LibraryUnitTests
    {
        [Theory]
        [InlineData(123, "02:03 min")]
        [InlineData(23, "23 sec")]
        [InlineData(3743, "01:02:23")]
        public void FormatDurationInSecondsHHMMSSReturnsCorrectResult(long durationInSeconds, string expectedResult)
        {
            var formatted = Toggl.FormatDurationInSecondsHHMMSS(durationInSeconds);
            Assert.Equal(expectedResult, formatted);
        }

        [Fact]
        public void ShapeOnLightBackground_ShouldConvertRgbToHsvWithoutAdaptation()
        {
            var green = new Toggl.TogglRgbColor {r = 0, b = 0, g = 1.0};
            var hsvColor = Toggl.GetAdaptiveHsvColor(green, Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground);
            Assert.Equal(120.0 / 360.0, hsvColor.h);
            Assert.Equal(1.0, hsvColor.s);
            Assert.Equal(1.0, hsvColor.v);
        }
    }
}