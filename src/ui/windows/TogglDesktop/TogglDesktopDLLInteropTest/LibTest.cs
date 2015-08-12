using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace TogglDesktopDLLInteropTest
{
    [TestClass]
    public class LibTest
    {
        [TestInitialize]
        public void Initialize()
        {
            Assert.IsTrue(TogglDesktop.Toggl.StartUI("0.0.0"));
        }

        [TestCleanup]
        public void Cleanup()
        {
            TogglDesktop.Toggl.Clear();
        }

        [TestMethod]
        public void TestStartUI()
        {
            
        }
    }
}
