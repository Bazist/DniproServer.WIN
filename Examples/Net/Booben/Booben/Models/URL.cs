using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class URL
    {
        public bool? IsActive { get; set; }
        public string Address { get; set; }

        public void Validate()
        {
            Validator.ValidateText(Address, "Address is not valid");
        }
    }
}