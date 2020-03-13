using System;
using System.Reactive.Linq;

namespace TogglDesktop.Services
{
    public class UpdateService : IDisposable
    {
        public UpdateService()
        {
            IsUpdateCheckEnabled = !Toggl.IsUpdateCheckDisabled();
        }

        public bool IsUpdateCheckEnabled { get; }
        public IObservable<UpdateStatus> UpdateStatus { get; } = Observable.Return(new UpdateStatus());

        public bool HasPendingUpdate { get; }
        public void UpdateAndQuit(int exitCode = 0){}
        public void UpdateAndRestart(){}

        public void Dispose()
        {
        }
    }

    public class UpdateStatus
    {
        public UpdateStatus()
        {
            HasUpdate = false;
        }

        public UpdateStatus(Version version, Toggl.DownloadStatus downloadStatus)
        {
            HasUpdate = true;
            Version = version;
            DownloadStatus = downloadStatus;
        }

        public bool HasUpdate { get; }
        public Version Version { get; }
        public Toggl.DownloadStatus DownloadStatus { get; }
    }
}