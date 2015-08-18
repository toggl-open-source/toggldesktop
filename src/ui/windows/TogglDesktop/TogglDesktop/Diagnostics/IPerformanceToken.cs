using System;

namespace TogglDesktop.Diagnostics
{
    interface IPerformanceToken : IDisposable
    {
        void Stop();
        IPerformanceToken WithInfo(string additionalInfo);
    }
}