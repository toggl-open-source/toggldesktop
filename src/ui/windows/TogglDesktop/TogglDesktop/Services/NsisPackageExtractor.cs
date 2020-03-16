using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Onova.Services;

namespace TogglDesktop.Services
{
    public class NsisPackageExtractor : IPackageExtractor
    {
        public async Task ExtractPackageAsync(string sourceFilePath, string destDirPath, IProgress<double>? progress = null,
            CancellationToken cancellationToken = new CancellationToken())
        {
            await Task.Run(() =>
            {
                var process = new Process();
                process.StartInfo.FileName = sourceFilePath;
                process.StartInfo.Arguments = $"/S /U /D={destDirPath}";
                process.StartInfo.UseShellExecute = false;
                process.Start();
                process.WaitForExit();
            }, cancellationToken);
        }
    }
}