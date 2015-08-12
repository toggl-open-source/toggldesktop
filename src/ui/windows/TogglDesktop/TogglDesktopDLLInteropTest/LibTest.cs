using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace TogglDesktopDLLInteropTest
{
    [TestClass]
    public class LibTest
    {
        [TestMethod]
        public void TestStartUI()
        {
            Assert.IsTrue(TogglDesktop.Toggl.StartUI("0.0.0"));
        }
    }
}
