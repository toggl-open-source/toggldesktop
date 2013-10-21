import atomac
import os
import os.path
import unittest

class TestOSXUI(unittest.TestCase):

    def setUp(self):
        cwd = os.getcwd()
        appPath = os.path.join(cwd, 'src/ui/osx/test2.project/build/Release/TogglDesktop.app')
        self.appLaunched = atomac.launchAppByBundlePath(appPath)

    def test_launched(self):
        self.assertTrue(self.appLaunched)

    def tearDown(self):
        atomac.terminateAppByBundleId('com.github.tanel.kopsik.TogglDesktop')

if __name__ == '__main__':
    unittest.main()