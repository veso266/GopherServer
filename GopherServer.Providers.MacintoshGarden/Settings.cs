using System.Configuration;

namespace GopherServer.Providers.MacintoshGarden
{
    public static class Settings
    {
        public static string HomePath
        {
            get
            {
                string Setting = MacintoshGardenProvider.providerConfig.AppSettings.Settings["MacintoshGarden.HomePath"].Value;
                if (!string.IsNullOrEmpty(Setting))
                    if (!Setting.StartsWith("/"))
                        Setting = "/" + Setting;
                return Setting;
            }
        }
    }
}
