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
    }
}