using System.Diagnostics;
using System.Globalization;
using System.Configuration;

namespace TogglDesktop.Diagnostics
{
    internal static class Performance
    {
        private static readonly bool debug = false;
        // ConfigurationSettings.AppSettings["Environment"] == "development"
        public static IPerformanceToken Measure(string activity)
        {
            if (debug)
            {
                return new PerformanceToken(activity);
            }

            return null;
        }

        #region Measure() overloads

        public static IPerformanceToken Measure(string template, object p0)
        {
            if (debug)
            {
                return Measure(string.Format(template, p0));
            }

            return null;
        }

        public static IPerformanceToken Measure(string template, object p0, object p1)
        {
            if (debug)
            {
                return Measure(string.Format(template, p0, p1));
            }

            return null;
        }

        public static IPerformanceToken Measure(string template, object p0, object p1, object p2)
        {
            if (debug)
            {
                return Measure(string.Format(template, p0, p1, p2));
            }

            return null;
        }

        #endregion

        private class PerformanceToken : IPerformanceToken
        {
            private readonly string _activity;
            private readonly Stopwatch _timer;

            private string _additionalInformation;

            public PerformanceToken(string activity)
            {
                Toggl.Debug(string.Format("Starting activity '{0}'", activity));
                this._activity = activity;
                this._timer = Stopwatch.StartNew();
            }

            public void Dispose()
            {
                this.Stop();
            }

            public void Stop()
            {
                lock (this._timer)
                {
                    if (!this._timer.IsRunning)
                        return;

                    this._timer.Stop();
                }

                Toggl.Debug(string.Format("Measured activity '{0}', took {1} ms",
                    this._activity + this._additionalInformation,
                    this._timer.Elapsed.TotalMilliseconds.ToString(CultureInfo.InvariantCulture)));
            }

            public IPerformanceToken WithInfoNotNull(string additionalInfo)
            {
                this._additionalInformation = this._additionalInformation + ", " + additionalInfo;
                return this;
            }
        }
    }
}
