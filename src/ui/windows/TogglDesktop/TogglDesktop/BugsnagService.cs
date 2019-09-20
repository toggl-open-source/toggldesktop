using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;
using Bugsnag;
using Bugsnag.Payload;
using Exception = System.Exception;

namespace TogglDesktop
{
public static class BugsnagService
{
    private static Client bugsnag;

    public static void Init()
    {
        Toggl.InitialiseLog();

        var configuration = new Configuration("aa13053a88d5133b688db0f25ec103b7");
        configuration.AppVersion = Program.Version();

#if TOGGL_PRODUCTION_BUILD
        configuration.ReleaseStage = "production";
#else
        configuration.ReleaseStage = "development";
#endif

        bugsnag = new Client(configuration);
        bugsnag.BeforeNotify(report =>
        {
            report.Event.User = new User
            {
                Id = Program.UserId.ToString()
            };
            report.Event.Metadata.Add("Details", new Dictionary<string, string>
            {
                {
                    "Channel", Toggl.UpdateChannel()
                },
                {
                    "Bitness", Utils.Bitness()
                },
                {
                    "InstallationType",
#if MS_STORE
                    "Store"
#else
                    "Regular"
#endif
                }
            });
        });

        Toggl.OnError += delegate(string errmsg, bool user_error)
        {
            Toggl.Debug(errmsg);
            try
            {
                if (!user_error && bugsnag.Configuration.ReleaseStage != "development")
                    NotifyBugsnag(new Exception(errmsg));
            }
            catch (Exception ex)
            {
                Toggl.Debug("Could not check if can notify bugsnag: " + ex);
            }
        };

        Application.ThreadException += Application_ThreadException;
        AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
    }

    private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
    {
        NotifyBugsnag(e.ExceptionObject as Exception);
    }

    private static void Application_ThreadException(object sender, ThreadExceptionEventArgs e)
    {
        NotifyBugsnag(e.Exception);
    }

    public static void NotifyBugsnag(Exception e)
    {
        Toggl.Debug("Notifying bugsnag: " + e);
        try
        {
            bugsnag.Notify(e);
        }
        catch (Exception ex)
        {
            Toggl.Debug("Could not notify bugsnag: " + ex);
        }
    }
}
}