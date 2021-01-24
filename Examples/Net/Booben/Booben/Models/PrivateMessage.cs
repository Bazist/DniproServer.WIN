using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class PrivateMessage
    {
        public bool? IsRead { get; set; }
        public DateTime? OnDate { get; set; }

        public string FromLogin { get; set; }
        public string ToLogin { get; set; }

        public string Message { get; set; }

        public void Validate()
        {
            Validator.ValidateName(FromLogin, "From login is not valid");
            Validator.ValidateName(ToLogin, "To login is not valid");

            Validator.ValidateText(Message, "Message is not valid");
        }
    }
}