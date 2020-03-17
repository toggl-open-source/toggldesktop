using System;
using System.Reactive.Linq;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using TogglDesktop.Services;

namespace TogglDesktop.ViewModels
{
    public class AboutWindowViewModel : ReactiveObject
    {
        private readonly UpdateService _updateService;
        public string[] Channels { get; } = {"stable", "beta", "dev"};

        public AboutWindowViewModel(UpdateService updateService, string versionText)
        {
            VersionText = versionText;
            _updateService = updateService;
            var updateStatus = updateService.UpdateStatus;
            IsUpdateCheckEnabled = updateService.IsUpdateCheckEnabled;
            if (IsUpdateCheckEnabled)
            {
                updateStatus.Select(GetUpdateStatusText)
                    .ToPropertyEx(this, x => x.UpdateStatusText);
            }
            UpdateAndRestartCommand = ReactiveCommand.Create(UpdateAndRestart,
                updateStatus
                    .Select(status => status.DownloadStatus == Toggl.DownloadStatus.Done)
                    .ObserveOnDispatcher());
        }

        public string VersionText { get; }

        public bool IsUpdateCheckEnabled { get; }

        [Reactive]
        public string SelectedChannel { get; set; }

        public string UpdateStatusText { [ObservableAsProperty] get; }

        public IReactiveCommand UpdateAndRestartCommand { get; }

        public void InstallPendingUpdate()
        {
            _updateService.InstallPendingUpdate();
        }

        private void UpdateAndRestart()
        {
            Toggl.PrepareShutdown();
            _updateService.InstallPendingUpdate();
            Program.Shutdown(0);
        }

        public void InitUpdateChannel(string channel)
        {
            SelectedChannel = channel;
            this.ObservableForProperty(x => x.SelectedChannel)
                .Select(x => x.Value)
                .Subscribe(SetUpdateChannel);
        }

        private static void SetUpdateChannel(string channel) => Toggl.SetUpdateChannel(channel);

        private static string GetUpdateStatusText(UpdateStatus status) =>
            status.HasUpdate
                ? (status.DownloadStatus == Toggl.DownloadStatus.Started
                    ? $"Downloading version {status.Version} ..."
                    : $"New version {status.Version} available!")
                : string.Empty;
    }
}