using System;
using System.Threading.Tasks;
using Windows.ApplicationModel;

namespace TogglDesktop.Win10
{
#if MS_STORE
    public static class RunOnStartup
    {
        /// <returns>
        /// <c>true</c> if enabled
        /// <c>false</c> if disabled but can be enabled from the app
        /// <c>null</c> if disabled and cannot be enabled from the app
        /// </returns>
        public static async Task<bool?> IsRunOnStartupEnabled()
        {
            var startupTask = await GetStartupTask();
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

        private static async Task DisableRunOnStartup()
        {
            var startupTask = await GetStartupTask();
            if (startupTask.State == StartupTaskState.Enabled)
            {
                startupTask.Disable();
            }
        }

        private static async Task<bool> TryEnableRunOnStartup()
        {
            var startupTask = await GetStartupTask();
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

        private static async Task<StartupTask> GetStartupTask() => await StartupTask.GetAsync("TogglDesktopStartupTask");
    }
#endif
}