using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace Bugsnag.WebTest.Controllers
{
    public class HomeController : Controller
    {
        public ActionResult Index()
        {
            ViewData["APIKeySet"] = false;
            if(ConfigurationManager.AppSettings["BugSnagApiKey"] != "YOUR_API_KEY_HERE")
            {
                ViewData["APIKeySet"] = true;
            }

            return View();
        }

        public ActionResult ThrowException()
        {
            throw new ApplicationException("This is a test exception from BugSnag.WebTest");
        }

    }
}
