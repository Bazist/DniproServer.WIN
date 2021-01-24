using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Condition
    {
        public bool? IsActive { get; set; }
        public bool? OnlyTitle { get; set; }

        public string Phrase { get; set; }
        public string Words { get; set; }
        public Reference Library { get; set; }

        public int? MinWords { get; set; }
        public bool? UseStemming { get; set; }

        public void Validate()
        {
            if (Library != null)
            {
                Library.Validate();
            }

            Validator.ValidateText(Phrase, "Phrase is not valid in condition");
            Validator.ValidateText(Words, "Words is not valid in condition");

            Validator.ValidateNumber(MinWords, "Min words is not valid.", 1, 100);
        }
    }
}