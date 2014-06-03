using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Http;
using System.Web.Mvc;
using System.Web.Routing;
using Bugsnag.Library;

namespace Bugsnag.WebTest
{
    // Note: For instructions on enabling IIS6 or IIS7 classic mode, 
    // visit http://go.microsoft.com/?LinkId=9394801
    public class MvcApplication : System.Web.HttpApplication
    {
        protected void Application_Start()
        {
            AreaRegistration.RegisterAllAreas();

            WebApiConfig.Register(GlobalConfiguration.Configuration);
            FilterConfig.RegisterGlobalFilters(GlobalFilters.Filters);
            RouteConfig.RegisterRoutes(RouteTable.Routes);
        }

        //  Added for the BugSnag sample web application
        protected void Application_Error(object sender, EventArgs e)
        {
            //  Create a new BugSnag notifier
            BugSnag bs = new BugSnag();

            //  Notify.  This will get configuration from the web.config
            //  and gather all known errors and report them.  It's just that simple!
            bs.Notify();
        }
    }
}