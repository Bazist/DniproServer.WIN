using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Home
    {
        public List<Item> TopItems { get; set; }
        public Robot[] TopRobots { get; set; }
    }
}