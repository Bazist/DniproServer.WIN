using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Source
    {
        public Source()
        {
            URLS = new List<URL>();
        }

        /// <summary>
        /// All, Urls, Sites, Agents, Google
        /// </summary>
        public string Type { get; set; }
        public List<URL> URLS { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Type, "Source type is not valid");

            foreach(URL url in URLS)
            {
                url.Validate();
            }
        }
    }
}