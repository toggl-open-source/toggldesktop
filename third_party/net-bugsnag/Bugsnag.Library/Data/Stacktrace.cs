using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Bugsnag.Library.Data
{
    /// <summary>
    /// Each stacktrace object represents one line in
    /// the exception's stacktrace. Bugsnag uses this information to help
    /// with error grouping, as well as displaying it to the user.
    /// </summary>
    [DataContract]
    public class Stacktrace
    {
        /// <summary>
        /// The file that this stack frame was executing.
        /// It is recommended that you strip any unnecessary or common
        /// information from the beginning of the path.
        /// </summary>
        [DataMember(Name = "file")]
        public string File
        {
            get;
            set;
        }

        /// <summary>
        /// The line of the file that this frame of the stack was in.
        /// </summary>
        [DataMember(Name = "lineNumber")]
        public int LineNumber
        {
            get;
            set;
        }

        /// <summary>
        /// The method that this particular stack frame is within.
        /// </summary>
        [DataMember(Name = "method")]
        public string Method
        {
            get;
            set;
        }

        /// <summary>
        /// Is this stacktrace line is in the user's project code, set 
        /// this to true. It is useful for developers to be able to see 
        /// which lines of a stacktrace are within their own application, 
        /// and which are within third party libraries. This boolean field
        /// allows Bugsnag to display this information in the stacktrace
        /// as well as use the information to help group errors better.
        /// (Optional, defaults to false).
        /// </summary>
        [DataMember(Name = "inProject")]
        public bool InProject
        {
            get;
            set;
        }
    }
}
