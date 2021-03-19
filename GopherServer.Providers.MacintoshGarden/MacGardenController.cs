﻿using System;
using System.Linq;
using System.Net;
using GopherServer.Core.Models;
using GopherServer.Core.Results;
using GopherServer.Providers.MacintoshGarden.Results;
using GopherServer.Providers.MacintoshGarden.Models;

namespace GopherServer.Providers.MacintoshGarden
{
    public class MacGardenController
    {
        private string[] validHosts = new string[] { "www.macintoshgarden.org", "macintoshgarden.org", "mirror.macintoshgarden.org" };

        public BaseResult ShowApp(string url) => new SoftwareResult(new SoftwareItem(url));

        internal DirectoryResult Search(string search)
        {
            var searchUrl = "http://macintoshgarden.org/search/node/" + WebUtility.UrlEncode(search + " type:app,game");
            var searchResults = new Models.SearchResults(searchUrl);
            return new Results.SearchResult(searchResults);
        }

        internal DirectoryResult SearchPage(string url)
        {   
            var searchResults = new Models.SearchResults(url);
            return new Results.SearchResult(searchResults);
        }

        public BaseResult DoDownload(string url)
        {
            var uri = new Uri(url);

            if (!validHosts.Contains(uri.Host))
                return new ErrorResult("Invalid host");

            return new ProxyResult(url, "http://macintoshgarden.org");
        }

        public DirectoryResult ShowHome()
        {
            var result = new DirectoryResult();
            result.Items.Add(new DirectoryItem("Macintosh Garden - Gopher Edition"));
            result.Items.Add(new DirectoryItem("================================="));
            result.Items.Add(new DirectoryItem(""));
            result.Items.Add(new DirectoryItem(ItemType.INDEXSEARCH, "Search the Garden", Settings.HomePath + "/search/"));

            return result;
        }
    }
}
