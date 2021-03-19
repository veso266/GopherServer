using System.Configuration;

namespace GopherServer.Providers.WpJson
{
    public static class Settings
    {
        public static string Url => ConfigurationManager.AppSettings["WordPressProvider.Url"];
        //public static string HomePath => ConfigurationManager.AppSettings["WordPressProvider.HomePath"];

        public static string HomePath { 
            get 
            { 
                string Setting = ConfigurationManager.AppSettings["WordPressProvider.HomePath"];
                if (!string.IsNullOrEmpty(Setting))
                    if (!Setting.StartsWith("/"))
                        Setting = "/" + Setting;
                return Setting;
            } 
        }
    }
}
