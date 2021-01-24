using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Subscriber
    {
        public bool? IsActive { get; set; }
        public string Login { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Login, "Login is not valid");
        }
    }
}