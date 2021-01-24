using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Comment
    {
        public Comment()
        {
        }

        public Author Author { get; set; }
        public string Text { get; set; }

        public void Validate()
        {
            if (Author != null)
            {
                Author.Validate();
            }

            Validator.ValidateText(Text, "Comment has forbidden symbols.");
        }
    }
}