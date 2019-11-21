using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Windows.Input;
using MahApps.Metro.Controls;
using ReactiveUI;
using ReactiveUI.Validation.Extensions;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class PreferencesWindowViewModel : ReactiveValidationObject<PreferencesWindowViewModel>
    {
        private readonly Dictionary<HotKey, string> _knownShortcuts =
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

        private const string ShowHideTogglDescription = "Show/Hide Toggl";
        private const string ContinueStopTimerDescription = "Continue/Stop Timer";

        private HotKey _showHideToggl;
        private HotKey _continueStopTimer;
        private HotKey _continueStopTimerSaved;
        private HotKey _showHideTogglSaved;

        private readonly ValidationHelper _showHideTogglValidation;
        private readonly ValidationHelper _continueStopTimerValidation;

        public PreferencesWindowViewModel()
        {
            this.WhenAnyValue(x => x.ShowHideToggl)
                .Buffer(2, 1)
                .Subscribe(b => UpdateKnownShortcuts(b[0], b[1], ShowHideTogglDescription));
            this.WhenAnyValue(x => x.ContinueStopTimer)
                .Buffer(2, 1)
                .Subscribe(b => UpdateKnownShortcuts(b[0], b[1], ContinueStopTimerDescription));

            _showHideTogglValidation = this.ValidationRule(
                vm => vm.ShowHideToggl,
                hotKey => IsHotKeyValid(hotKey, ShowHideTogglDescription),
                hotKey => $"This shortcut is already taken by {_knownShortcuts[hotKey]}");
            _continueStopTimerValidation = this.ValidationRule(
                vm => vm.ContinueStopTimer,
                hotKey => IsHotKeyValid(hotKey, ContinueStopTimerDescription),
                hotKey => $"This shortcut is already taken by {_knownShortcuts[hotKey]}");
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

        public void ResetPropsWithErrorsToPreviousValues()
        {
            if (!_showHideTogglValidation.IsValid) ShowHideToggl = _showHideTogglSaved;
            if (!_continueStopTimerValidation.IsValid) ContinueStopTimer = _continueStopTimerSaved;
        }

        private bool IsHotKeyValid(HotKey hotKey, string hotKeyDescription)
        {
            return hotKey.IsNullOrNone() || _knownShortcuts[hotKey] == hotKeyDescription;
        }

        private void UpdateKnownShortcuts(HotKey previousValue, HotKey newValue, string hotKeyDescription)
        {
            if (!previousValue.IsNullOrNone())
            {
                if (_knownShortcuts[previousValue] == hotKeyDescription)
                {
                    _knownShortcuts.Remove(previousValue);
                }
            }
            if (!newValue.IsNullOrNone())
            {
                if (!_knownShortcuts.ContainsKey(newValue))
                {
                    _knownShortcuts.Add(newValue, hotKeyDescription);
                }
            }
        }

        private static HotKey LoadHotKey(Func<Key> getKey, Func<ModifierKeys> getModifiers)
        {
            var key = getKey();
            return new HotKey(key, key == Key.None ? ModifierKeys.None : getModifiers());
        }
    }
}