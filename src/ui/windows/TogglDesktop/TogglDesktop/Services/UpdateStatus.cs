using System;

namespace TogglDesktop.Services
{
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