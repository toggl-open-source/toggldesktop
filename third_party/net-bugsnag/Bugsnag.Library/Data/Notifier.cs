using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Bugsnag.Library.Data
{
    /// <summary>
    /// Notifier meta data
    /// </summary>
    [DataContract]
    public class Notifier
    {
        public Notifier()
        {
            Name = ".NET Bugsnag";
            Version = "0.1";
            Url = "https://github.com/danesparza/net-bugsnag";
        }

        /// <summary>
        /// The notifier name
        /// </summary>
        [DataMember(Name="name")]
        public string Name
        {
            get;
            set;
        }

        /// <summary>
        /// The notifier's current version
        /// </summary>
        [DataMember(Name = "version")]
        public string Version
        {
            get;
            set;
        }

        /// <summary>
        /// The URL associated with the notifier
        /// </summary>
        [DataMember(Name = "url")]
        public string Url
        {
            get;
            set;
        }
    }
}
