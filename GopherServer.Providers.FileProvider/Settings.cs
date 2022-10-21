using GopherServer.Core.Configuration;
using System.Collections.Generic;
using System.Configuration;

namespace GopherServer.Providers.FileProvider
{
    public static class Settings
    {
        public static string RootDirectory => FileProvider.providerConfig.AppSettings.Settings["FileProvider.RootDirectory"].Value;
    }
}
