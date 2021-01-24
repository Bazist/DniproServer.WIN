using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Notification
    {
        public bool? IsActive { get; set; }
        public string Type { get; set; }
        public string Address { get; set; }
        public DateTime? OnDate { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Type, "Type is not valid");

            Validator.ValidateText(Address, "Address is not valid");
        }
    }
}