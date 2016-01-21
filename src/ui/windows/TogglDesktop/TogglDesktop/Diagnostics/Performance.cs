
using System.Diagnostics;
using System.Globalization;

namespace TogglDesktop.Diagnostics
{
    static class Performance
    {
        public static IPerformanceToken Measure(string activity)
        {
            return null;//new PerformanceToken(activity);
        }

        #region Measure() overloads

        public static IPerformanceToken Measure(string template, object p0)
        {
            return null;//Measure(string.Format(template, p0));
        }
        public static IPerformanceToken Measure(string template, object p0, object p1)
        {
            return null;//Measure(string.Format(template, p0, p1));
        }
        public static IPerformanceToken Measure(string template, object p0, object p1, object p2)
        {
            return null;//Measure(string.Format(template, p0, p1, p2));
        }

        #endregion

        private class PerformanceToken : IPerformanceToken
        {
            private readonly string activity;
            private readonly Stopwatch timer;

            private string additionalInformation;

            public PerformanceToken(string activity)
            {
                Toggl.Debug(string.Format("Starting activity '{0}'", activity));
                this.activity = activity;
                this.timer = Stopwatch.StartNew();
            }

            public void Dispose()
            {
                this.Stop();
            }

            public void Stop()
            {
                lock (this.timer)
                {
                    if (!this.timer.IsRunning)
                        return;

                    this.timer.Stop();
                }

                Toggl.Debug(string.Format("Measured activity '{0}', took {1} ms",
                    this.activity + this.additionalInformation,
                    this.timer.Elapsed.TotalMilliseconds.ToString(CultureInfo.InvariantCulture)));
            }

            public IPerformanceToken WithInfoNotNull(string additionalInfo)
            {
                this.additionalInformation = this.additionalInformation + ", " + additionalInfo;
                return this;
            }
        }
    }
}
