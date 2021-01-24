using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Author
    {
        public Author()
        {
            Likes = new List<Like>();
        }

        public bool? IsPrivate { get; set; }
        public bool? IsActive { get; set; }

        public string Login { get; set; }
        public List<Like> Likes { get; set; }
        public DateTime? OnDate { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Login, "Login is not correct.");
        }
    }
}