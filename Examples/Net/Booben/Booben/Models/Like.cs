using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Like
    {
        public DateTime? OnDate { get; set; }
        public string Login { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Login, "Login is not valid");
        }
    }
}