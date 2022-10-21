﻿using System;
using System.Linq;
using System.Reflection;
using GopherServer.Core.Configuration;

namespace GopherServer.Core.Providers
{
    public class ProviderFactory
    {
        /// <summary>
        /// Gets the provider specified
        /// </summary>
        /// <param name="hostname"></param>
        /// <param name="port"></param>
        /// <returns></returns>
        public static IServerProvider[] GetProviders(string hostname, int port)
        {
            // TODO: plugins could maybe load by itself (located in providers dir)
            IServerProvider[] providers = new IServerProvider[ServerSettings.Providers.Count];

            for (int i = 0; i < ServerSettings.Providers.Count; i++)
            {
                var assemblyName = ServerSettings.Providers[i].name;

                var asm = Assembly.Load(assemblyName);
                if (asm == null)
                    throw new TypeLoadException("Unable to find any assembly named '" + assemblyName + "'");

                var type = asm.GetTypes().FirstOrDefault(x => typeof(IServerProvider).IsAssignableFrom(x));
                if (type == null)
                    throw new TypeLoadException("No IServerProvider found in " + asm.FullName);

                providers[i] = (IServerProvider)Activator.CreateInstance(type, hostname, port);
            }

            return providers;
        }
    }
}
