using System;

namespace TogglDesktop.Diagnostics
{
    interface IPerformanceToken : IDisposable
    {
        void Stop();
        void Stop(string additionalInformation);
    }
}