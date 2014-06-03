using System;
using System.Collections.Generic;
using System.Configuration;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using Bugsnag.Library.Data;
using ServiceStack.Text;

namespace Bugsnag.Library
{
    /// <summary>
    /// .NET notifier for BugSnag error reporting
    /// </summary>
    public class BugSnag
    {
        /// <summary>
        /// Http based url for reporting errors to BugSnag
        /// </summary>
        private string httpUrl = "http://notify.bugsnag.com";

        /// <summary>
        /// Https based url for reporting errors to BugSnag
        /// </summary>
        private string httpsUrl = "https://notify.bugsnag.com";

        /// <summary>
        /// Creates a bugsnag notifier and sets the API key
        /// </summary>
        /// <param name="apiKey"></param>
        public BugSnag(string apiKey) : this()
        {
            this.apiKey = apiKey;
        }

        /// <summary>
        /// Constructor to set defaults
        /// </summary>
        public BugSnag()
        {
            //  SSL is set to 'off' by default
            useSSL = false;

            //  Release stage defaults to 'production'
            releaseStage = "production";

            //  Notify release stages defaults to just notifying 
            //  for production
            notifyReleaseStages = new List<string>();
            notifyReleaseStages.Add("production");

            //  CHECK CONFIGURATION SETTINGS

            //  apiKey
            if(!string.IsNullOrEmpty(ConfigurationManager.AppSettings["BugSnagApiKey"]))
            {
                apiKey = ConfigurationManager.AppSettings["BugSnagApiKey"];
            }

            //  SSL
            if(!string.IsNullOrEmpty(ConfigurationManager.AppSettings["BugSnagUseSSL"]))
            {
                useSSL = Convert.ToBoolean(ConfigurationManager.AppSettings["BugSnagUseSSL"]);
            }

            //  Release stage
            if(!string.IsNullOrEmpty(ConfigurationManager.AppSettings["BugSnagReleaseStage"]))
            {
                releaseStage = ConfigurationManager.AppSettings["BugSnagReleaseStage"];
            }

            //  Notify release stages
            if(!string.IsNullOrEmpty(ConfigurationManager.AppSettings["BugSnagNotifyReleaseStages"]))
            {
                notifyReleaseStages.Clear();
                notifyReleaseStages.AddRange(ConfigurationManager.AppSettings["BugSnagNotifyReleaseStages"].Split('|'));
            }

            //  Application version
            if(!string.IsNullOrEmpty(ConfigurationManager.AppSettings["applicationVersion"]))
            {
                applicationVersion = ConfigurationManager.AppSettings["applicationVersion"];
            }
        }

        /// <summary>
        /// The apiKey for the project
        /// </summary>
        public string apiKey
        {
            get;
            set;
        }

        /// <summary>
        /// The current release stage for the application 
        /// (development/test/production)
        /// </summary>
        public string releaseStage
        {
            get;
            set;
        }

        /// <summary>
        /// A list of release stages that the notifier will capture and send 
        /// errors for. If the current release stage is not in this list, errors 
        /// should not be sent to Bugsnag. 
        /// </summary>
        public List<string> notifyReleaseStages
        {
            get;
            set;
        }

        /// <summary>
        /// If this is true, the plugin should notify Bugsnag using SSL
        /// </summary>
        public bool useSSL
        {
            get;
            set;
        }

        /// <summary>
        /// The version number of the application which generated the error
        /// </summary>
        public string applicationVersion
        {
            get;
            set;
        }

        /// <summary>
        /// The operating system version of the client that the error was 
        /// generated on.
        /// </summary>
        public string OSVersion
        {
            get;
            set;
        }

        /// <summary>
        /// Gathers information for the last error (if any error is available) 
        /// and reports it to BugSnag using information from the application
        /// configuration file and other defaults
        /// </summary>
        public void Notify()
        {
            Notify(null);
        }

        /// <summary>
        /// Gathers information for the last error (if any error is available) 
        /// and reports it to BugSnag using information from the application
        /// configuration file and other defaults
        /// </summary>
        /// <param name="extraData">Any extra data to pass when reporting this error</param>
        public void Notify(object extraData)
        {
            //  If we're a web application, we can report errors automagically
            if(HttpContext.Current != null)
            {
                //  If we have errors...
                if (HttpContext.Current.AllErrors != null && HttpContext.Current.AllErrors.Any())
                {
                    //  ... go through all of the errors and report them
                    List<Event> events = new List<Event>();
                    events.Add(ProcessExceptions(
                        HttpContext.Current.AllErrors.ToList(),
                        HttpContext.Current.Request.Path,
                        GetDefaultUserId(),
                        extraData)
                    );

                    //  Send the notification:
                    ErrorNotification notification = new ErrorNotification()
                    {
                        Api_Key = this.apiKey,
                        Events = events
                    };

                    SendNotification(notification, this.useSSL);
                }
            }

            //  If we're not a web application, we're SOL ATM (call another method)
        }

        /// <summary>
        /// Report a single exception to BugSnag using defaults
        /// </summary>
        /// <param name="ex">The exception to report</param>
        /// <param name="extraData">Data that will be sent as meta-data along with this error</param>
        public void Notify(System.Exception ex, object extraData) 
        {
            Notify(ex, string.Empty, string.Empty, extraData);
        }

        /// <summary>
        /// Report a list of exceptions to BugSnag
        /// </summary>
        /// <param name="exList">The list of Exceptions to report</param>
        /// <param name="extraData">Data that will be sent as meta-data along with this error</param>
        public void Notify(List<System.Exception> exList, object extraData)
        {
            Notify(exList, string.Empty, string.Empty, extraData);
        }

        /// <summary>
        /// Report an exception to Bugsnag with other per-request or per-session data
        /// </summary>
        /// <param name="ex">The exception to report</param>
        /// <param name="userId">An ID representing the current application's user.  If this isn't set
        /// this defaults to sessionId if available</param>
        /// <param name="context">The context that is currently active in the application</param>
        /// <param name="extraData">Data that will be sent as meta-data along with this error</param>
        public void Notify(System.Exception ex, string userId, string context, object extraData)
        {
            List<System.Exception> exList = new List<System.Exception>();
            exList.Add(ex);

            Notify(exList, userId, context, extraData);
        }

        /// <summary>
        /// Report a list of exceptions to Bugsnag with other per-request or per-session data
        /// </summary>
        /// <param name="exList">The list of exceptions to report</param>
        /// <param name="userId">An ID representing the current application's user.  If this isn't set
        /// this defaults to sessionId if available</param>
        /// <param name="context">The context that is currently active in the application</param>
        /// <param name="extraData">Data that will be sent as meta-data along with every error</param>
        public void Notify(List<System.Exception> exList, string userId, string context, object extraData)
        {
            //  Add an event for this exception list:
            List<Event> events = new List<Event>();
            events.Add(ProcessExceptions(exList, context, userId, extraData));

            //  Send the notification:
            ErrorNotification notification = new ErrorNotification()
            {
                Api_Key = this.apiKey,
                Events = events
            };

            SendNotification(notification, this.useSSL);
        }

        /// <summary>
        /// Gets the default UserId to use when reporting errors
        /// </summary>
        /// <returns></returns>
        private string GetDefaultUserId()
        {
            string retval = string.Empty;

            //  First, check to see if we have an HttpContext to work with
            if(HttpContext.Current != null)
            {
                //  If we have a current user, use that
                if(!string.IsNullOrEmpty(HttpContext.Current.User.Identity.Name))
                {
                    retval = HttpContext.Current.User.Identity.Name;
                }
                else if(HttpContext.Current.Session != null)
                {
                    //  Otherwise, use sessionID
                    retval = HttpContext.Current.Session.SessionID ?? String.Empty;
                }
            }

            //  If not, we're SOL
            return retval;
        }

        /// <summary>
        /// Process a list of exceptions into an event
        /// </summary>
        /// <param name="exList">A list of exceptions</param>
        /// <param name="Context">The context for the event</param>
        /// <param name="UserId">The userId for the event</param>
        /// <param name="ExtraData">Extra data to annotate on the event</param>
        /// <returns></returns>
        private Event ProcessExceptions(List<System.Exception> exList, string Context, string UserId, object extraData)
        {
            //  Create an event to return
            Event retval = new Event()
            {
                AppVersion = this.applicationVersion,
                Context = Context,
                OSVersion = this.OSVersion,
                ReleaseStage = this.releaseStage,
                UserId = UserId,
                ExtraData = extraData
            };

            //  Our list of exceptions:
            List<Bugsnag.Library.Data.Exception> exceptions = new List<Bugsnag.Library.Data.Exception>();

            //  For each exception passed...
            foreach(System.Exception ex in exList)
            {
                //  ... Create a list of stacktraces
                //  This may not be the best way to get this information:
                //  http://blogs.msdn.com/b/jmstall/archive/2005/03/20/399287.aspx
                var stacktraces = (from item in new System.Diagnostics.StackTrace(ex, true).GetFrames()
                                   select new Stacktrace()
                                   {
                                       File = item.GetFileName() ?? item.GetType().Name ?? "N/A",
                                       LineNumber = item.GetFileLineNumber(),
                                       Method = item.GetMethod().Name
                                   }).ToList();

                //  Add a new exception, and use the stacktrace list:
                exceptions.Add(new Bugsnag.Library.Data.Exception()
                {
                    ErrorClass = ex.TargetSite.Name,
                    Message = ex.Message,
                    Stacktrace = stacktraces
                });
            }

            //  Set our list of exceptions
            retval.Exceptions = exceptions;

            //  Return the event:
            return retval;
        }

        /// <summary>
        /// Sends current set of events to BugSnag via a JSON post
        /// </summary>
        /// <param name="notification">The notification to send</param>
        /// <param name="useSSL">Indicates the post should use SSL when sending JSON data</param>
        private void SendNotification(ErrorNotification notification, bool useSSL)
        {
            string serializedJSON = notification.SerializeToString();

            //  Create a byte array:
            byte[] byteArray = Encoding.UTF8.GetBytes(serializedJSON);

            //  Post JSON to server:
            WebRequest request;
            if(useSSL)
                request = WebRequest.Create(httpsUrl);
            else
                request = WebRequest.Create(httpUrl);

            request.Method = WebRequestMethods.Http.Post;
            request.ContentType = "application/json";
            request.ContentLength = byteArray.Length;

            Stream dataStream = request.GetRequestStream();
            dataStream.Write(byteArray, 0, byteArray.Length);
            dataStream.Close();

            //  Get the response.  See https://bugsnag.com/docs/notifier-api for response codes
            var response = request.GetResponse();
        }
    }
}
