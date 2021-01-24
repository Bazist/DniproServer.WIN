using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Library
    {
        public Library()
        {
            Comments = new List<Comment>();
            Subscribers = new List<Subscriber>();
        }

        public Author Author { get; set; }

        public Reference Reference { get; set; }

        public string Category { get; set; }
        public string SubCategory { get; set; }

        public string Name { get; set; }
        public string Description { get; set; }

        public string Words { get; set; }

        public List<Comment> Comments { get; set; }
        public List<Subscriber> Subscribers { get; set; }

        public int GetAmountNewWords()
        {
            return 0; // ApproveWords.Count(w => !w.IsRead);
        }

        public void Validate()
        {
            if (Author != null)
            {
                Author.Validate();
            }

            if (Reference != null)
            {
                Reference.Validate();
            }

            Validator.ValidateName(Category, "Category is not valid");
            Validator.ValidateName(SubCategory, "SubCategory is not valid");
            Validator.ValidateName(Name, "Name is not valid");

            Validator.ValidateText(Description, "Description is not valid");
            Validator.ValidateText(Words, "Description is not valid");

            foreach(Comment com in Comments)
            {
                com.Validate();
            }

            foreach(Subscriber sub in Subscribers)
            {
                sub.Validate();
            }
        }
    }
}