using System;
using System.Reactive.Linq;
using DynamicData.Binding;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using TogglDesktop.Services;

namespace TogglDesktop.ViewModels
{
    public class AboutWindowViewModel : ReactiveObject
    {
        private readonly UpdateService _updateService;
        public string[] Channels { get; } = {"stable", "beta", "dev"};

        public AboutWindowViewModel(UpdateService updateService)
        {
            VersionText = $"Version {Program.Version()} {Utils.Bitness()}";
            _updateService = updateService;
            var updateStatus = updateService.UpdateStatus;
            IsUpdateCheckEnabled = updateService.IsUpdateCheckEnabled;
            if (IsUpdateCheckEnabled)
            {
                updateStatus.Select(GetUpdateStatusText)
                    .ToPropertyEx(this, x => x.UpdateStatusText);
                this.WhenPropertyChanged(x => x.SelectedChannel)
                    .Select(x => x.Value)
                    .Subscribe(SetUpdateChannel);
            }
            UpdateAndRestartCommand = ReactiveCommand.Create(UpdateAndRestart,
                updateStatus.Select(status => status.DownloadStatus == Toggl.DownloadStatus.Done));
        }

        public string VersionText { get; }

        public bool IsUpdateCheckEnabled { get; }

        [Reactive]
        public string SelectedChannel { get; set; }

        public string UpdateStatusText { [ObservableAsProperty] get; }

        public IReactiveCommand UpdateAndRestartCommand { get; }

        private void UpdateAndRestart()
        {
            Toggl.PrepareShutdown();
            _updateService.Update();
            Program.Shutdown(0);
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