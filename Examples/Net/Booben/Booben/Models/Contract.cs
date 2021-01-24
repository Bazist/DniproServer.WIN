using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Contract
    {
        public int? MaxGroups { get; set; }
        public int? MaxRobots { get; set; }
        public int? MaxLibraries { get; set; }
        public int? MaxWordsInLib { get; set; }

        public void Validate()
        {
            
        }
    }
}