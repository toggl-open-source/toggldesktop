using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Onova.Services;

namespace TogglDesktop.Services
{
    public class NsisPackageExtractor : IPackageExtractor
    {
        public async Task ExtractPackageAsync(string sourceFilePath, string destDirPath, IProgress<double> progress = null,
            CancellationToken cancellationToken = new CancellationToken())
        {
            await Task.Run(() =>
            {
                var installerPath = sourceFilePath;
                var exePath = Path.ChangeExtension(sourceFilePath, ".exe");
                try
                {
                    File.Move(sourceFilePath, exePath);
                    installerPath = exePath;
                }
                catch (Exception e) when (e is IOException || e is UnauthorizedAccessException)
                {
                    BugsnagService.NotifyBugsnag(e);
                }

                try
                {
                    var process = new Process
                    {
                        StartInfo =
                        {
                            FileName = installerPath,
                            Arguments = $"/S /autoupdate /D={destDirPath}",
                            UseShellExecute = false
                        }
                    };
                    process.Start();
                    process.WaitForExit();
                }
                catch (Exception e) when (e is Win32Exception || e is IOException || e is UnauthorizedAccessException)
                {
                    BugsnagService.NotifyBugsnag(e);
                }
            }, cancellationToken);
        }
    }
}