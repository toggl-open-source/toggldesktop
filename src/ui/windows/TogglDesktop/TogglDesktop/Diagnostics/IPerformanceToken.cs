using System;

namespace TogglDesktop.Diagnostics
{
    internal interface IPerformanceToken : IDisposable
    {
        void Stop();
        IPerformanceToken WithInfoNotNull(string additionalInfo);
    }

    internal static class Extensions
    {
        public static IPerformanceToken WithInfo(this IPerformanceToken token, string additionalInfo)
        {
            return token == null ? null : token.WithInfoNotNull(additionalInfo);
        }
    }
}