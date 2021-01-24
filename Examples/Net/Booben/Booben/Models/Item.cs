using DniproClient;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Item
    {
        public Item()
        {
        }

        public Author Author { get; set; }
        public string Title { get; set; }
        public string URL { get; set; }
        public string Text { get; set; }
        public string Encoding { get; set; }
        public string FileEncoding { get; set; }
        public int? Index { get; set; }

        public void Validate()
        {
            if (Author != null)
            {
                Author.Validate();
            }

            Validator.ValidateText(Title, "Title is not valid");
            Validator.ValidateText(URL, "Title is not valid");
            Validator.ValidateText(Text, "Title is not valid");
        }

        public string GetDecodedText()
        {
            return Serialization.Decode(Text);
        }
    }
}