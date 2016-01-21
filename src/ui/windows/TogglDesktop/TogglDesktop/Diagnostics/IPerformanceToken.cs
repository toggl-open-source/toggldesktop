using System;

namespace TogglDesktop.Diagnostics
{
    interface IPerformanceToken : IDisposable
    {
        void Stop();
        IPerformanceToken WithInfoNotNull(string additionalInfo);
    }

    static class Extensions
    {
        public static IPerformanceToken WithInfo(this IPerformanceToken token, string additionalInfo)
        {
            return token == null ? null : token.WithInfoNotNull(additionalInfo);
        }
    }
}