using System;
using System.Linq;
using System.Reflection;
using Microsoft.Win32;

namespace TogglDesktop.Services
{
    public static class DeepLinkProtocolInstaller
    {
        public const string StartupUri = "togglauth";
        private const string _ssoLogin = "sso-login";

        public static void InstallProtocol()
        {
            try
            {
                var regLocation = Registry.CurrentUser.OpenSubKey("Software", true).OpenSubKey("Classes", true);
                var key = regLocation.OpenSubKey(StartupUri, true) ?? regLocation.CreateSubKey(StartupUri);
                key?.SetValue("URL Protocol", _ssoLogin);
                var commandKey = key?.OpenSubKey(@"shell\open\command", true) ?? key?.CreateSubKey(@"shell\open\command");
                commandKey?.SetValue("", Assembly.GetExecutingAssembly().Location + " %1");
            }
            catch (Exception e)
            {
                BugsnagService.NotifyBugsnag(e);
            }
        }
    }
}
