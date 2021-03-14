namespace GopherServer
{
    public class Program
    {
        public static void Main(string[] args)
        {
			//Show config file location
#if DEBUG
            Console.WriteLine("Config file location: " + System.Configuration.ConfigurationManager.OpenExeConfiguration(System.Configuration.ConfigurationUserLevel.None).FilePath);
#endif
            var server = new Server();
            server.StartListening();
        }
    }
}
