using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class WhatEvent
    {
        public string Type { get; set; }
        public string Login { get; set; }
        public string Message { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Type, "Type is not valid");
            Validator.ValidateName(Login, "Login is not valid");
            Validator.ValidateText(Message, "Message is not valid");
        }
    }
}