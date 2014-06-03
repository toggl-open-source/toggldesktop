using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Bugsnag.Library.Data
{
    /// <summary>
    /// Error event that Bugsnag should be notified of. A notifier
    /// can choose to group notices into an array to minimize network traffic, or
    /// can notify Bugsnag each time an event occurs
    /// </summary>
    [DataContract]
    public class Event
    {
        public Event()
        {
            Exceptions = new List<Exception>();
        }

        /// <summary>
        /// A unique identifier for a user affected by this event. This could be 
        /// any distinct identifier that makes sense for your application/platform.
        /// This field is optional but highly recommended.
        /// </summary>
        [DataMember(Name = "userId")]
        public string UserId
        {
            get;
            set;
        }

        /// <summary>
        /// The version number of the application which generated the error.
        /// (optional, default none)
        /// </summary>
        [DataMember(Name = "appVersion")]
        public string AppVersion
        {
            get;
            set;
        }

        /// <summary>
        /// The operating system version of the client that the error was 
        /// generated on. (optional, default none)
        /// </summary>
        [DataMember(Name = "osVersion")]
        public string OSVersion
        {
            get;
            set;
        }

        /// <summary>
        /// The release stage that this error occurred in, for example 
        /// "development" or "production". This can be any string, but "production"
        /// will be highlighted differently in bugsnag in the future, so please use
        /// "production" appropriately.
        /// </summary>
        [DataMember(Name = "releaseStage")]
        public string ReleaseStage
        {
            get;
            set;
        }

        /// <summary>
        /// A string representing what was happening in the application at the 
        /// time of the error. This string could be used for grouping purposes, 
        /// depending on the event.
        /// Usually this would represent the controller and action in a server 
        /// based project. It could represent the screen that the user was 
        /// interacting with in a client side project.
        /// For example,
        ///   * On Ruby on Rails the context could be controller#action
        ///   * In Android, the context could be the top most Activity.
        ///   * In iOS, the context could be the name of the top most UIViewController
        /// </summary>
        [DataMember(Name = "context")]
        public string Context
        {
            get;
            set;
        }

        /// <summary>
        /// An array of exceptions that occurred during this event. Most of the
        /// time there will only be one exception, but some languages support 
        /// "nested" or "caused by" exceptions. In this case, exceptions should 
        /// be unwrapped and added to the array one at a time. The first exception
        /// raised should be first in this array.
        /// </summary>
        [DataMember(Name = "exceptions")]
        public List<Exception> Exceptions
        {
            get;
            set;
        }

        /// <summary>
        /// An object containing any further data you wish to attach to this error
        /// event. This should contain one or more objects, with each object being
        /// displayed in its own tab on the event details on the Bugsnag website.
        /// (Optional).
        /// </summary>
        [DataMember(Name = "metaData")]
        public object ExtraData
        {
            get;
            set;
        }
    }
}
