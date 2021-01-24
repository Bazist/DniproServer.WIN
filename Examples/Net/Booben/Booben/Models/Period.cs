using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Period
    {
        /// <summary>
        /// all, week, month, day, year, unread, from-to
        /// </summary>
        public string Type { get; set; }
        public DateTime? FromDate { get; set; }
        public DateTime? ToDate { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Type, "Type is not valid.");
        }
    }
}