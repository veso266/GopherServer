using System.Configuration;

namespace GopherServer.Providers.Rss
{
    public static class Settings
    {
        public static string HomePath
        {
            get
            {
                string Setting = ConfigurationManager.AppSettings["RSS.HomePath"];
                if (!string.IsNullOrEmpty(Setting))
                    if (!Setting.StartsWith("/"))
                        Setting = "/" + Setting;
                return Setting;
            }
        }
    }
}
