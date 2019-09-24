using System;
using System.Threading.Tasks;
using Windows.ApplicationModel;

namespace TogglDesktop.Win10
{
#if MS_STORE
    public static class RunOnStartup
    {
        public static async Task<bool?> IsRunOnStartupEnabled()
        {
            var startupTask = await StartupTask.GetAsync("TogglDesktopStartupTask");
            switch (startupTask.State)
            {
                case StartupTaskState.DisabledByPolicy:
                    // disabled by group policy or not supported on this device
                    // cannot be changed by the user
                    return null;
                case StartupTaskState.DisabledByUser:
                    // disabled by the user in Task Manager
                    // cannot be enabled from the app, only from Task Manager
                    return null;
                case StartupTaskState.Disabled:
                    return false;
                case StartupTaskState.Enabled:
                    return true;
                default:
                    return null;
            }
        }

        public static async Task<bool> TrySetRunOnStartup(bool isEnabled)
        {
            if (isEnabled)
            {
                return await TryEnableRunOnStartup();
            }
            else
            {
                await DisableRunOnStartup();
                return true;
            }
        }

        public static async Task DisableRunOnStartup()
        {
            var startupTask = await StartupTask.GetAsync("TogglDesktopStartupTask");
            if (startupTask.State == StartupTaskState.Enabled)
            {
                startupTask.Disable();
            }
        }

        public static async Task<bool> TryEnableRunOnStartup()
        {
            var startupTask = await StartupTask.GetAsync("TogglDesktopStartupTask");
            if (startupTask.State == StartupTaskState.Disabled)
            {
                var newState = await startupTask.RequestEnableAsync();
                if (newState == StartupTaskState.Enabled)
                {
                    return true;
                }
            }
            return false;
        }
    }
#endif
}