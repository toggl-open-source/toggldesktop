using System;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Reactive.Threading.Tasks;
using System.Threading.Tasks;
using Onova;
using Onova.Services;

namespace TogglDesktop.Services
{
    public class UpdateService : IDisposable
    {
        private readonly IUpdateManager _updateManager;
        private IObservable<bool> _prepareUpdateObservable;
        private readonly Subject<UpdateStatus> _updateStatusSubject = new Subject<UpdateStatus>();
        public UpdateService()
        {
            IsUpdateCheckEnabled = !Toggl.IsUpdateCheckDisabled();
            _updateManager = new UpdateManager(
                new LocalPackageResolver("updates", "TogglDesktopInstaller*.exe"),
                new NsisPackageExtractor());

            Toggl.OnUpdateDownloadStatus
                .Where(x => x.DownloadStatus != Toggl.DownloadStatus.Done)
                .Subscribe(_updateStatusSubject);
            Toggl.OnUpdateDownloadStatus
                .Where(x => x.DownloadStatus == Toggl.DownloadStatus.Done)
                .Subscribe(PrepareUpdate);
            _updateStatusSubject.Subscribe(x => this.HasPendingUpdate = x.DownloadStatus == Toggl.DownloadStatus.Done);
            UpdateStatus = _updateStatusSubject.AsObservable();
        }

        public bool IsUpdateCheckEnabled { get; }
        public IObservable<UpdateStatus> UpdateStatus { get; }
        public bool HasPendingUpdate { get; private set; }

        public void Update(bool withRestart = true)
        {
            var lastPreparedVersion = _updateManager.GetPreparedUpdates().LastOrDefault();
            if (lastPreparedVersion != null)
            {
                _updateManager.LaunchUpdater(lastPreparedVersion, withRestart);
            }
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
                _prepareUpdateObservable.Where(x => x).Subscribe(_ => _updateStatusSubject.OnNext(updateStatus));
            }
        }

        private async Task<bool> PrepareUpdateAsync()
        {
            var check = await _updateManager.CheckForUpdatesAsync();
            if (check.CanUpdate)
            {
                await _updateManager.PrepareUpdateAsync(check.LastVersion);
                return true;
            }

            return false;
        }
    }
}