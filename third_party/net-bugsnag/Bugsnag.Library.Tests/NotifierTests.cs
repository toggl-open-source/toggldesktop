using System;
using Bugsnag.Library.Data;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ServiceStack.Text;
using System.Collections.Generic;
using System.Net;
using System.IO;
using System.Text;

namespace Bugsnag.Library.Tests
{
    [TestClass]
    public class NotifierTests
    {
        [TestMethod]
        public void TestAutoNotifier()
        {

            try
            {
                throw new ApplicationException("Throwing an app extension.  You heartless bastard.");
            }
            catch(System.Exception ex)
            {
                BugSnag bs = new BugSnag()
                {
                    apiKey = "YOUR_API_KEY"
                };

                bs.Notify(ex, new
                {
                    OtherReallyCoolData = new
                    {
                        color = "Yellow",
                        mood = "Mellow"
                    }
                });
            }
        }
    }
}
