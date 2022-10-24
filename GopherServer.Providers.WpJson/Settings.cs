using System.Configuration;

namespace GopherServer.Providers.WpJson
{
    public static class Settings
    {
        public static string Url => WordPressProvider.providerConfig.AppSettings.Settings["WordPressProvider.Url"].Value;
        //public static string HomePath => ConfigurationManager.AppSettings["WordPressProvider.HomePath"];

        public static string HomePath { 
            get 
            {
                string Setting = null;
                try
                {
                    Setting = WordPressProvider.providerConfig.AppSettings.Settings["WordPressProvider.HomePath"].Value;
                }
                catch {}
                if (!string.IsNullOrEmpty(Setting))
                    if (!Setting.StartsWith("/"))
                        Setting = "/" + Setting;
                return Setting;
            } 
        }
    }
}
