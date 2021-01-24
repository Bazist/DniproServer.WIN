using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Output
    {
        /// <summary>
        /// Text, Chart, Video, Coub, Image, Gif, Table
        /// </summary>
        public string Type { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Type, "Type is not valid");
        }
    }
}