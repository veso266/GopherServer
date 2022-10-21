using System.Configuration;

namespace GopherServer.Providers.Rss
{
    public static class Settings
    {
        public static string HomePath
        {
            get
            {
                string Setting = RssProvider.providerConfig.AppSettings.Settings["RSS.HomePath"].Value;
                if (!string.IsNullOrEmpty(Setting))
                    if (!Setting.StartsWith("/"))
                        Setting = "/" + Setting;
                return Setting;
            }
        }
    }
}
