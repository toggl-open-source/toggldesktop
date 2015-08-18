
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
        public static IPerformanceToken Measure(string template, object p0, object p1)
        {
            return Measure(string.Format(template, p0, p1));
        }

        #endregion

        private class PerformanceToken : IPerformanceToken
        {
            private readonly string activity;
            private readonly Stopwatch timer;

            private string additionalInformation;

            public PerformanceToken(string activity)
            {
                Console.WriteLine("Starting activity '{0}'", activity);
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

                Console.WriteLine("Measured activity '{0}', took {1} ms",
                    this.activity + this.additionalInformation, this.timer.Elapsed.TotalMilliseconds);
            }

            public IPerformanceToken WithInfo(string additionalInfo)
            {
                this.additionalInformation = this.additionalInformation + ", " + additionalInfo;
                return this;
            }
        }
    }
}
