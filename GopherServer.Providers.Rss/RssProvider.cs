using System;
using System.Collections.Generic;
using System.Linq;
using GopherServer.Core.Providers;
using GopherServer.Core.Results;
using GopherServer.Core.Routes;
using GopherServer.Providers.Rss.Data;

namespace GopherServer.Providers.Rss
{
    /// <summary>
    /// Rss Provider
    /// </summary>
    public class RssProvider : ServerProviderBase
    {
        public static System.Configuration.Configuration providerConfig = System.Configuration.ConfigurationManager.OpenExeConfiguration(System.Reflection.Assembly.GetExecutingAssembly().Location);
        Db db;

        public RssProvider(string hostname, int port) : base(hostname, port)
        { }

        public List<Route> Routes { get; set; }

        public RssController Controller { get; private set; }

        public override void Init()
        {
            db = new Db("rss.db");
            this.Controller = new RssController(db);
            this.Routes = BuildRoutes();

            var feedDownloader = new System.Threading.Timer(e => { Syndication.Syndication.UpdateFeeds(db); }, null, 0, (int)TimeSpan.FromMinutes(5).TotalMilliseconds);
        }

        private List<Route> BuildRoutes()
        {
            var routes = new List<Route>()
            {
                // Named groups require that the parameter name matches the group name
                // User Feed Listing
                new NamedGroupRoute("UserFeeds", Settings.HomePath + @"\/feeds\/(?<nickname>\w+)\/$", new Func<string, BaseResult>(this.Controller.GetUserFeeds)),

                // User Feed Listing (ie 'login')
                new NamedGroupRoute("UserFeedsQuery", Settings.HomePath + @"\/feeds\/\t(?<nickname>.*)", new Func<string, BaseResult>(this.Controller.GetUserFeeds)),

                // Combined view of the user's feeds
                new NamedGroupRoute("CombinedUserFeeds", Settings.HomePath + @"\/feeds\/(?<nickname>\w+)\/all\/$", new Func<string, BaseResult>(this.Controller.GetCombinedFeeds)),

                // View of selected Feed
                new NamedGroupRoute("SpecificUserFeed", Settings.HomePath + @"\/feeds\/(?<nickname>\w+)\/(?<feedId>\d+)\/$", new Func<string, int, BaseResult>(this.Controller.GetFeed)),

                // View Feed Item
                new NamedGroupRoute("FeedItem", Settings.HomePath + @"\/feeds\/(?<nickname>\w+)\/(?<feedId>\d+)\/(?<itemId>.+)", new Func<string, int, string, BaseResult>(this.Controller.GetFeedItem)),

                // Registration
                new NamedGroupRoute("Registration", Settings.HomePath + @"\/register\/*\t(?<nickname>\w+)", new Func<string, BaseResult>(this.Controller.RegisterUser)),

                // Add Feed
                new NamedGroupRoute("AddFeed", Settings.HomePath + @"\/user\/(?<nickname>\w+)\/add\/\t(?<feedUrl>.+)", new Func<string, string, BaseResult>(this.Controller.AddFeed)),

                // Delete User Feed
                new NamedGroupRoute("DeleteFeed", Settings.HomePath + @"\/user\/(?<nickname>.\w+)\/delete\/(?<feedId>\d+)\/$", new Func<string, int, BaseResult>(this.Controller.DeleteFeed)),
            };
            
            return routes;
        }

        public override BaseResult GetResult(string selector)
        {
            try
            {
                if (selector == (Settings.HomePath + "/") && !string.IsNullOrEmpty(Settings.HomePath)) // 
                    return Controller.GetHomePage();
                else if ((string.IsNullOrEmpty(selector) || selector == "1" || selector == "/") && (string.IsNullOrEmpty(Settings.HomePath) || Settings.HomePath == "/")) //If HomePath is not defined | some clients seem to use 1 or /
                    return Controller.GetHomePage();

                // Check our routes
                var route = this.Routes.FirstOrDefault(r => r.IsMatch(selector));

                if (route == null)
                    return new ErrorResult("Selector '" + selector + "' was not found/is not supported.");
                else
                {
                    Console.WriteLine("Matched Route: {0}", route.Name);
                    return route.Execute(selector);
                }
            }
            catch (Exception ex)
            {
                // TODO: Some kind of common logging?
                Console.WriteLine(ex);
                return new ErrorResult("Error occurred processing your request.");
            }
        }
    }
}
