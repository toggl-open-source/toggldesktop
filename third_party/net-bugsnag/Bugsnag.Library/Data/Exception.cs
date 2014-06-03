using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Bugsnag.Library.Data
{
    /// <summary>
    /// Exception that occurred during this event. Most of the
    /// time there will only be one exception, but some languages support 
    /// "nested" or "caused by" exceptions. In this case, exceptions should 
    /// be unwrapped and added to the exceptions array one at a time. 
    /// </summary>
    [DataContract]
    public class Exception
    {
        /// <summary>
        /// The class of error that occurred. This field is used to group the
        /// errors together so should not contain any contextual information
        /// that would prevent correct grouping. This would ordinarily be the
        /// Exception name when dealing with an exception.
        /// </summary>
        [DataMember(Name = "errorClass")]
        public string ErrorClass
        {
            get;
            set;
        }

        /// <summary>
        /// The error message associated with the error. Usually this will 
        /// contain some information about this specific instance of the error
        /// and is not used to group the errors (optional, default none).
        /// </summary>
        [DataMember(Name = "message")]
        public string Message
        {
            get;
            set;
        }

        /// <summary>
        /// An array of stacktrace objects. Each object represents one line in
        /// the exception's stacktrace. Bugsnag uses this information to help
        /// with error grouping, as well as displaying it to the user.
        /// </summary>
        [DataMember(Name = "stacktrace")]
        public List<Stacktrace> Stacktrace
        {
            get;
            set;
        }
    }
}
