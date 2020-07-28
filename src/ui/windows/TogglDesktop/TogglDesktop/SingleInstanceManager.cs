using System;

namespace TogglDesktop
{
    public class SingleInstanceManager<T> : Microsoft.VisualBasic.ApplicationServices.WindowsFormsApplicationBase
        where T : System.Windows.Application, new()
    {
        private T _app;
        public SingleInstanceManager()
        {
            IsSingleInstance = true;
        }

        public event Action BeforeStartup;

        protected override bool OnStartup(Microsoft.VisualBasic.ApplicationServices.StartupEventArgs e)
        {
            BeforeStartup?.Invoke();
            _app = new T();
            _app.Run();
            return false;
        }

        protected override void OnStartupNextInstance(Microsoft.VisualBasic.ApplicationServices.StartupNextInstanceEventArgs e)
        {
            _app.MainWindow?.ShowOnTop();
        }
    }
}
