
using System;
using System.Diagnostics;

namespace TogglDesktop.Diagnostics
{
    static class Performance
    {
        public static IPerformanceToken Measure(string activity)
        {
            return new PerformanceToken(activity);
        }

        #region Measure() overloads

        public static IPerformanceToken Measure(string template, object p0)
        {
            return Measure(string.Format(template, p0));
        }

        #endregion

        private class PerformanceToken : IPerformanceToken
        {
            private readonly string activity;
            private readonly Stopwatch timer;

            public PerformanceToken(string activity)
            {
                this.activity = activity;
                this.timer = Stopwatch.StartNew();
            }

            public void Dispose()
            {
                this.Stop();
            }

            public void Stop()
            {
                this.Stop(null);
            }

            public void Stop(string additionalInformation)
            {
                lock (this.timer)
                {
                    if (!this.timer.IsRunning)
                        return;

                    this.timer.Stop();
                }

                var template = string.IsNullOrWhiteSpace(additionalInformation)
                    ? "Measured activity '{0}', took {1} ms"
                    : "Measured activity '{0}, {2}', took {1} ms";

                Console.WriteLine(template, this.activity, this.timer.Elapsed.TotalMilliseconds, additionalInformation);
            }
        }
    }
}
