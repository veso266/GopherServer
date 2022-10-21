using System.Collections.Generic;
using System.Configuration;
using System.Xml;


namespace GopherServer.Core.Configuration
{
    class ProvidersConfigSection : IConfigurationSectionHandler
    {
        public object Create(object parent, object configContext, XmlNode section)
        {
            List<Provider> myConfigObject = new List<Provider>();

            foreach (XmlNode childNode in section.ChildNodes)
            {
                foreach (XmlAttribute attrib in childNode.Attributes)
                {
                    myConfigObject.Add(new Provider() { name = attrib.Value });
                }
            }
            return myConfigObject;
        }
    }
    public class Provider
    {
        public string name { get; set; }
    }
}
