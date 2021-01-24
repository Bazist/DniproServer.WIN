using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Globalization;
using System.Web.Mvc;
using System.Web.Security;
using System.Text;

namespace BigDocForum.Models
{
    public class Message
    {
        public int ArticleID { get; set; }
        public string Author { get; set; }
        public long CreatedDate { get; set; }
        public string Text { get; set; }

        public void Validate()
        {
            if (string.IsNullOrEmpty(Author) ||
                string.IsNullOrEmpty(Text) ||
                Author.Length > 256 ||
                Text.Length > 256 ||
                Author.Contains("'") ||
                Text.Contains("\""))
            {
                throw new Exception("Validation error");
            }
        }
    }

    public class Article
    {
        public void Validate()
        {
            if (string.IsNullOrEmpty(Author) ||
                string.IsNullOrEmpty(Title) ||
                string.IsNullOrEmpty(Text) ||
                Author.Length > 256 ||
                Title.Length > 256 ||
                Text.Length > 256 ||
                Author.Contains("'") ||
                Title.Contains("'") ||
                Text.Contains("\""))
            {
                throw new Exception("Validation error");
            }
        }

        public uint ID { get; set; }

        public string Author { get; set; }
        public string Title { get; set; }
        public string LastAuthor { get; set; }
        public long CreatedDate { get; set; }
        public long UpdatedDate { get; set; }
        public int CurrentPage { get; set; }

        public int CountMessage { get; set; }

        public string Text { get; set; }

        public List<Message> Messages { get; set; }
    }
}
