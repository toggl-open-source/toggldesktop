using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Reactive.Threading.Tasks;
using System.Reflection;
using System.Threading.Tasks;
using Onova;
using Onova.Exceptions;
using Onova.Models;
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
                AssemblyMetadata.FromAssembly(Assembly.GetEntryAssembly() ?? Assembly.GetAssembly(typeof(UpdateService))),
                new LocalPackageResolver(updatesPath, "TogglDesktopInstaller*.exe"),
                new NsisPackageExtractor());

            Toggl.OnUpdateDownloadStatus
                .Where(x => x.DownloadStatus != Toggl.DownloadStatus.Done)
                .Subscribe(_updateStatusSubject);
            Toggl.OnUpdateDownloadStatus
                .Where(x => x.DownloadStatus == Toggl.DownloadStatus.Done)
                .Subscribe(PrepareUpdate);
            UpdateStatus = _updateStatusSubject.AsObservable();
            UpdateChannel = new BehaviorSubject<string>("stable");
            UpdateChannel.DistinctUntilChanged().Skip(1).Subscribe(channel => Toggl.SetUpdateChannel(channel));
        }

        public bool IsUpdateCheckEnabled { get; }
        public IObservable<UpdateStatus> UpdateStatus { get; }
        public BehaviorSubject<string> UpdateChannel { get; }

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

        // ReSharper disable once UnusedMember.Global
        public void InstallPendingUpdatesOnStartup()
        {
            if (InstallPendingUpdate())
            {
                // quit, updater will restart the app
                Process.GetCurrentProcess().Kill();
            }

            DeleteOldUpdates();
        }

        private static void DeleteOldUpdates()
        {
            Directory.CreateDirectory(Toggl.UpdatesPath); // make sure the directory exists
            var di = new DirectoryInfo(Toggl.UpdatesPath);
            foreach (var file in di.GetFiles("TogglDesktopInstaller*.exe", SearchOption.TopDirectoryOnly))
            {
                try
                {
                    Utils.DeleteFile(file.FullName);
                }
                catch (Exception e)
                {
                    BugsnagService.NotifyBugsnag(e);
                    Toggl.NewError($"Unable to delete the file: {file.FullName}. Delete this file manually.", false);
                }
            }
            var updatesDir = new DirectoryInfo(
                Path.Combine(
                    Environment.GetFolderPath(
                        Environment.SpecialFolder.LocalApplicationData), "Onova", "TogglDesktop"));
            if (updatesDir.Exists)
            {
                foreach (var file in updatesDir.GetFiles("*.exe", SearchOption.TopDirectoryOnly))
                {
                    try
                    {
                        Utils.DeleteFile(file.FullName);
                    }
                    catch (Exception e)
                    {
                        BugsnagService.NotifyBugsnag(e);
                        Toggl.NewError($"Unable to delete the file: {file.FullName}. Delete this file manually.", false);
                    }
                }
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