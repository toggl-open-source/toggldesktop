using System;
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

        [Theory]
        [InlineData("00FF00", 0.0, 1.0, 0.0)]
        [InlineData("FF0000", 1.0, 0.0, 0.0)]
        [InlineData("0000FF", 0.0, 0.0, 1.0)]
        [InlineData("000000", 0.0, 0.0, 0.0)]
        [InlineData("FFFFFF", 1.0, 1.0, 1.0)]
        [InlineData("808080", .5019, .5019, .5019)]
        public void ShapeOnLightBackground_ShouldConvertHexToRgbWithoutAdaptation(string hex, double r, double g, double b)
        {
            var rgbColor = Toggl.GetAdaptiveRgbColorFromHex(hex,Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground);
            Assert.Equal(r, rgbColor.r, 3);
            Assert.Equal(g, rgbColor.g, 3);
            Assert.Equal(b, rgbColor.b, 3);
        }

        [Theory]
        [InlineData("0B83D9", "096BB2", "2EA0F2", "30A9FF")]
        [InlineData("9E5BD9", "824AB2", "BA7AF2", "C381FF")]
        [InlineData("D94182", "B2356B", "F2619F", "FF66A8")]
        [InlineData("E36A00", "BC5800", "F5811A", "FF851B")]
        [InlineData("BF7000", "985900", "E9A13A", "F6AA3D")]
        [InlineData("2DA608", "227F06", "76E155", "7DEE5A")]
        [InlineData("06A893", "048171", "52E2CF", "57EEDB")]
        [InlineData("C9806B", "A26756", "ECA995", "F9B29D")]
        [InlineData("465BB3", "37478C", "8396E5", "8A9EF2")]
        [InlineData("990099", "720072", "DC58DC", "E95DE9")]
        [InlineData("C7AF14", "A08D10", "ECD646", "F9E14A")]
        [InlineData("566614", "353F0C", "BFCC8A", "CBD893")]
        [InlineData("D92B2B", "B22323", "F24C4C", "FF5050")]
        [InlineData("525266", "33333F", "BCBCCC", "C7C7D8")]
        [InlineData("991102", "720C01", "DC675A", "E96D5F")]
        public void GetAdaptiveRgbColorFromHex_ShouldConvertToCorrectColor(string shapeLight, string textLight, string shapeDark, string textDark)
        {
            var rgbShapeLight = Toggl.GetAdaptiveRgbColorFromHex(shapeLight,Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground);
            var expectedRgbShapeLight = HexToRgb(shapeLight);
            Assert.True(Math.Abs(expectedRgbShapeLight.r - rgbShapeLight.r) < 0.005);
            Assert.True(Math.Abs(expectedRgbShapeLight.g - rgbShapeLight.g) < 0.005);
            Assert.True(Math.Abs(expectedRgbShapeLight.b - rgbShapeLight.b) < 0.005);

            var rgbTextLight = Toggl.GetAdaptiveRgbColorFromHex(shapeLight,Toggl.TogglAdaptiveColor.AdaptiveColorTextOnLightBackground);
            var expectedRgbTextLight = HexToRgb(textLight);
            Assert.True(Math.Abs(expectedRgbTextLight.r - rgbTextLight.r) < 0.005);
            Assert.True(Math.Abs(expectedRgbTextLight.g - rgbTextLight.g) < 0.005);
            Assert.True(Math.Abs(expectedRgbTextLight.b - rgbTextLight.b) < 0.005);

            var rgbShapeDark = Toggl.GetAdaptiveRgbColorFromHex(shapeLight,Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnDarkBackground);
            var expectedRgbShapeDark = HexToRgb(shapeDark);
            Assert.True(Math.Abs(expectedRgbShapeDark.r - rgbShapeDark.r) < 0.005);
            Assert.True(Math.Abs(expectedRgbShapeDark.g - rgbShapeDark.g) < 0.005);
            Assert.True(Math.Abs(expectedRgbShapeDark.b - rgbShapeDark.b) < 0.005);

            var rgbTextDark = Toggl.GetAdaptiveRgbColorFromHex(shapeLight,Toggl.TogglAdaptiveColor.AdaptiveColorTextOnDarkBackground);
            var expectedRgbTextDark = HexToRgb(textDark);
            Assert.True(Math.Abs(expectedRgbTextDark.r - rgbTextDark.r) < 0.005);
            Assert.True(Math.Abs(expectedRgbTextDark.g - rgbTextDark.g) < 0.005);
            Assert.True(Math.Abs(expectedRgbTextDark.b - rgbTextDark.b) < 0.005);
        }

        private static Toggl.TogglRgbColor HexToRgb(string hex)
        {
            return new Toggl.TogglRgbColor
            {
                r = byte.Parse(hex.Substring(0, 2), System.Globalization.NumberStyles.HexNumber) / 255.0,
                g = byte.Parse(hex.Substring(2, 2), System.Globalization.NumberStyles.HexNumber) / 255.0,
                b = byte.Parse(hex.Substring(4, 2), System.Globalization.NumberStyles.HexNumber) / 255.0,
            };
        }
    }
}