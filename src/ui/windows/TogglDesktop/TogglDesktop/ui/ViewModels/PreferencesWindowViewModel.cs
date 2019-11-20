using System;
using System.Collections.Generic;
using System.Windows.Input;
using MahApps.Metro.Controls;
using ReactiveUI;
using ReactiveUI.Validation.Extensions;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class PreferencesWindowViewModel : ReactiveValidationObject<PreferencesWindowViewModel>
    {
        private static readonly Dictionary<HotKey, string> KnownShortcuts =
            new Dictionary<HotKey, string>
            {
                { new HotKey(Key.N, ModifierKeys.Control), "New time entry" },
                { new HotKey(Key.O, ModifierKeys.Control), "Continue last entry" },
                { new HotKey(Key.S, ModifierKeys.Control), "Stop time entry" },
                { new HotKey(Key.W, ModifierKeys.Control), "Hide Toggl Desktop" },
                { new HotKey(Key.R, ModifierKeys.Control), "Sync" },
                { new HotKey(Key.E, ModifierKeys.Control), "Edit time entry" },
                { new HotKey(Key.D, ModifierKeys.Control), "Toggle manual mode" },
                { new HotKey(Key.V, ModifierKeys.Control), "New time entry from clipboard" },
            };

        private HotKey _showHideToggl;
        private HotKey _continueStopTimer;
        private HotKey _continueStopTimerSaved;
        private HotKey _showHideTogglSaved;

        public PreferencesWindowViewModel()
        {
            this.ValidationRule(
                vm => vm.ShowHideToggl,
                hotKey => hotKey == null || (!KnownShortcuts.ContainsKey(hotKey)),
                hotKey => $"This shortcut is already taken by {KnownShortcuts[hotKey]}");
            this.ValidationRule(
                vm => vm.ContinueStopTimer,
                hotKey => hotKey == null || (!KnownShortcuts.ContainsKey(hotKey)),
                hotKey => $"This shortcut is already taken by {KnownShortcuts[hotKey]}");
        }

        public HotKey ShowHideToggl
        {
            get => _showHideToggl;
            set => this.RaiseAndSetIfChanged(ref _showHideToggl, value);
        }

        public HotKey GetShowHideTogglIfChanged() =>
            !object.Equals(ShowHideToggl, _showHideTogglSaved) ? (ShowHideToggl ?? new HotKey(Key.None)) : null;

        public HotKey ContinueStopTimer
        {
            get => _continueStopTimer;
            set => this.RaiseAndSetIfChanged(ref _continueStopTimer, value);
        }

        public HotKey GetContinueStopTimerIfChanged() =>
            !object.Equals(ContinueStopTimer, _continueStopTimerSaved) ? (ContinueStopTimer ?? new HotKey(Key.None)) : null;

        public void ResetRecordedShortcuts()
        {
            ShowHideToggl = _showHideTogglSaved;
            ContinueStopTimer = _continueStopTimerSaved;
        }

        public void LoadShortcutsFromSettings()
        {
            _showHideTogglSaved = LoadHotKey(Toggl.GetKeyShow, Toggl.GetKeyModifierShow);
            ShowHideToggl = _showHideTogglSaved;
            _continueStopTimerSaved = LoadHotKey(Toggl.GetKeyStart, Toggl.GetKeyModifierStart);
            ContinueStopTimer = _continueStopTimerSaved;
        }

        private static HotKey LoadHotKey(Func<Key> getKey, Func<ModifierKeys> getModifiers)
        {
            var key = getKey();
            return new HotKey(key, key == Key.None ? ModifierKeys.None : getModifiers());
        }
    }
}