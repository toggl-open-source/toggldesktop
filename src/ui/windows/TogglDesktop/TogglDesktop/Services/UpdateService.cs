using System;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Reactive.Threading.Tasks;
using System.Threading.Tasks;
using Onova;
using Onova.Exceptions;
using Onova.Services;

namespace TogglDesktop.Services
{
    public class UpdateService : IDisposable
    {
        private readonly IUpdateManager _updateManager;
        private IObservable<bool> _prepareUpdateObservable;
        private readonly Subject<UpdateStatus> _updateStatusSubject = new Subject<UpdateStatus>();
        public UpdateService(bool isUpdateCheckDisabled, string updatesPath)
        {
            IsUpdateCheckEnabled = !isUpdateCheckDisabled;
            _updateManager = new UpdateManager(
                new LocalPackageResolver(updatesPath, "TogglDesktopInstaller*.exe"),
                new NsisPackageExtractor());

            Toggl.OnUpdateDownloadStatus
                .Where(x => x.DownloadStatus != Toggl.DownloadStatus.Done)
                .Subscribe(_updateStatusSubject);
            Toggl.OnUpdateDownloadStatus
                .Where(x => x.DownloadStatus == Toggl.DownloadStatus.Done)
                .Subscribe(PrepareUpdate);
            UpdateStatus = _updateStatusSubject.AsObservable();
        }

        public bool IsUpdateCheckEnabled { get; }
        public IObservable<UpdateStatus> UpdateStatus { get; }

        public bool InstallPendingUpdate(bool withRestart = true)
        {
            var lastPreparedVersion = _updateManager.GetPreparedUpdates().LastOrDefault();
            if (lastPreparedVersion != null)
            {
                try
                {
                    _updateManager.LaunchUpdater(lastPreparedVersion, withRestart);
                    return true;
                }
                catch (Exception e)
                    when (e is UpdaterAlreadyLaunchedException || e is LockFileNotAcquiredException)
                {
                    // Ignore race conditions
                }
            }

            return false;
        }
        public void Dispose()
        {
            _updateManager.Dispose();
        }

        private void PrepareUpdate(UpdateStatus updateStatus)
        {
            if (_prepareUpdateObservable == null)
            {
                _prepareUpdateObservable = PrepareUpdateAsync().ToObservable();
                _prepareUpdateObservable
                    .Select(x => x ? updateStatus : new UpdateStatus())
                    .Subscribe(_updateStatusSubject);
            }
        }

        private async Task<bool> PrepareUpdateAsync()
        {
            var check = await _updateManager.CheckForUpdatesAsync();
            if (check.CanUpdate)
            {
                try
                {
                    await _updateManager.PrepareUpdateAsync(check.LastVersion);
                }
                catch (Exception e)
                    when (e is UpdaterAlreadyLaunchedException || e is LockFileNotAcquiredException)
                {
                    // Ignore race conditions
                    return false;
                }

                return true;
            }

            return false;
        }
    }
}