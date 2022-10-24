﻿using System;
using System.Collections.Generic;
using System.Linq;
using GopherServer.Core.Providers;
using GopherServer.Core.Results;
using GopherServer.Core.Routes;

namespace GopherServer.Providers.MacintoshGarden
{
    public class MacintoshGardenProvider : ServerProviderBase
    {
        public static System.Configuration.Configuration providerConfig = System.Configuration.ConfigurationManager.OpenExeConfiguration(System.Reflection.Assembly.GetExecutingAssembly().Location);

        public List<Route> Routes { get; private set; }
        public MacGardenController Controller { get; set; }

        public MacintoshGardenProvider(string hostname, int port) : base(hostname, port)
        { }        

        public override void Init()
        {
            Controller = new MacintoshGarden.MacGardenController();

            this.Routes = new List<Route>() {                
                new TypedRoute<string>("Bin", "BIN:(.+)", Controller.DoDownload), // Proxy Download                
                new TypedRoute<string>("Search", Settings.HomePath + @"\/search\/*\t(.+)", Controller.Search), // Search
                new TypedRoute<string>("Search", Settings.HomePath + @"\/search\/(http://macintoshgarden.org\/.+)", Controller.SearchPage),
                new TypedRoute<string>("App", Settings.HomePath + @"\/app\/(.+)", Controller.ShowApp), // App Result
                PrebuiltRoutes.GifRoute(), // Screenshot
            };
        }

        /// <summary>
        /// Processes the selector and performs the appropriate action
        /// </summary>
        /// <param name="selector"></param>
        /// <returns></returns>
        public override BaseResult GetResult(string selector)
        {
            // This is where we read our selectors...
            // it's a shame we can't reuse the route code out of MVC (or can we ?)

            try
            {
                if (selector == (Settings.HomePath + "/") && !string.IsNullOrEmpty(Settings.HomePath)) // 
                    return Controller.ShowHome();
                else if ((string.IsNullOrEmpty(selector) || selector == "1" || selector == "/") && (string.IsNullOrEmpty(Settings.HomePath) || Settings.HomePath == "/")) //If HomePath is not defined | some clients seem to use 1 or /
                    return Controller.ShowHome();

                // Check our routes
                var route = Routes.FirstOrDefault(r => r.IsMatch(selector));

                if (route == null)
                    return new ErrorResult("Selector '" + selector + "' was not found/is not supported.");
                else
                    return route.Execute(selector);
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
