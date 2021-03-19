#if DEBUG
using System;
#endif
using System.IO;
using System.Linq;
using GopherServer.Core.Providers;
using GopherServer.Core.Results;

namespace GopherServer.Providers.FileProvider
{
    public class FileProvider : ServerProviderBase
    {
        public FileProvider(string hostname, int port) : base(hostname, port)
        { }

        public string BaseDirectory { get; set; }

        public override void Init() => this.BaseDirectory = Settings.RootDirectory;

        public override BaseResult GetResult(string selector)
        {

#if DEBUG
            Console.Clear();
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine(selector);
#endif
            string path;
            if (selector == "/") 
            {
                selector = "";
                path = Path.Combine(BaseDirectory, selector);
            }
            else
                path = Path.Combine(BaseDirectory, selector);
            var indexPath = Path.Combine(path, "index.gopher");

            if (string.IsNullOrEmpty(selector) || selector == "/") //If we are at index, some clients like to use "" (netscape) or / (OverbiteNX firefox addon) as root selectors
            {
                if (File.Exists(indexPath))
                    return new TextResult(File.ReadAllLines(indexPath).ToList());
                else
                    return new DirectoryListingResult(BaseDirectory, BaseDirectory);
            }
            if (selector.Contains(".."))
                return new ErrorResult("Invalid Path");

            selector = selector.TrimStart(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);


            if (File.Exists(indexPath))
                return new TextResult(File.ReadAllLines(indexPath).ToList());

            if (File.Exists(path))
            {
                return new FileResult(path, BaseDirectory);
            }

            if (Directory.Exists(path))
                return new DirectoryListingResult(path, BaseDirectory);

            return new ErrorResult("Invalid Path");
        }       
    }
}
