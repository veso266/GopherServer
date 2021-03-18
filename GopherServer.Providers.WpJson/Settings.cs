using System.Configuration;

namespace GopherServer.Providers.WpJson
{
    public static class Settings
    {
        public static string Url => ConfigurationManager.AppSettings["WordPressProvider.Url"];
    }
}
