using System.Configuration;

namespace GopherServer.Providers.MacintoshGarden
{
    public static class Settings
    {
        public static string HomePath
        {
            get
            {
                string Setting = ConfigurationManager.AppSettings["MacintoshGarden.HomePath"];
                if (!string.IsNullOrEmpty(Setting))
                    if (!Setting.StartsWith("/"))
                        Setting = "/" + Setting;
                return Setting;
            }
        }
    }
}
