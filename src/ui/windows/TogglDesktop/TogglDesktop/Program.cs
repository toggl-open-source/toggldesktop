using System;
using System.Diagnostics;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Reflection;
using System.Windows.Interop;
using System.Windows.Media;
using Application = System.Windows.Forms.Application;

namespace TogglDesktop
{
static class Program
{
    private static SingleInstanceManager<App> singleInstanceManager;
    private static readonly BehaviorSubject<ulong> UserIdSubject = new BehaviorSubject<ulong>(0);
    public static ulong UserId => UserIdSubject.Value;
    public static bool IsLoggedIn => UserId > 0;

    [STAThread]
    static void Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        RenderOptions.ProcessRenderMode = RenderMode.SoftwareOnly;
        singleInstanceManager = new SingleInstanceManager<App>();
        singleInstanceManager.BeforeStartup += OnBeforeStartup;
        singleInstanceManager.Run(args);
    }

    private static void OnBeforeStartup()
    {
        Toggl.OnLogin.Select(x => x.userId).Subscribe(UserIdSubject);
        BugsnagService.Init();
        singleInstanceManager.BeforeStartup -= OnBeforeStartup;
    }

    public static void Shutdown(int exitCode)
    {
        Toggl.Clear();
        Environment.Exit(exitCode);
    }

    public static string Version()
    {
        var assembly = Assembly.GetExecutingAssembly();
        var versionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
        return $"{versionInfo.ProductMajorPart}.{versionInfo.ProductMinorPart}.{versionInfo.ProductBuildPart}";
    }
}
}
