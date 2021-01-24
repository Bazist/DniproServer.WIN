using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Friend
    {
        public bool? IsActive { get; set; }
        public DateTime? OnDate { get; set; }
        public bool? IsRequested { get; set; }
        public bool? IsApproved { get; set; }
        public string RequestFromLogin { get; set; }
        public string Login { get; set; }
    }
}