using Booben.Controllers;
using DniproClient;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;

namespace Booben.Models
{
    public class NewLike
    {
        public object Item { get; set; }
        public List<Like> Likes { get; set; }
    }

    public class User
    {
        public User()
        {
            //Author = new Author();

            //Contract = new Contract();

            Groups = new List<Group>();
            Favourits = new List<Item>();
            Libraries = new List<Library>();
            BanList = new List<Author>();

            PrivateMessages = new List<PrivateMessage>();

            Friends = new List<Friend>();
            Events = new List<Event>();
        }

        public Author Author { get; set; }

        public string Login { get; set; }
        public string Password { get; set; }
        public string From { get; set; }
        public string Email { get; set; }
        public byte[] Picture { get; set; }
        public string Sex { get; set; }
        public int? Age { get; set; }
        public string Interests { get; set; }
        public string SecKey { get; set; }

        public Contract Contract { get; set; }

        public List<Group> Groups { get; set; }
        public List<Item> Favourits { get; set; }
        public List<Library> Libraries { get; set; }
        public List<Author> BanList { get; set; }

        public List<PrivateMessage> PrivateMessages { get; set; }
        
        public List<Friend> Friends { get; set; }
        public List<Event> Events { get; set; }

        public void Validate()
        {
            if (Author != null)
            {
                Author.Validate();
            }

            Validator.ValidateName(Login, "Login is not valid");
            Validator.ValidateText(Password, "Password is not valid");
            Validator.ValidateText(From, "From is not valid");
            Validator.ValidateEmail(Email, "Email is not valid");
            Validator.ValidateName(Sex, "Sex is not valid");
            Validator.ValidateNumber(Age, "Age is not valid", 3, 150);
            Validator.ValidateText(Interests, "Interests is not valid");

            foreach (Group group in Groups)
            {
                group.Validate();
            }

            foreach (Item item in Favourits)
            {
                item.Validate();
            }

            foreach (Library lib in Libraries)
            {
                lib.Validate();
            }

            foreach (Author auth in BanList)
            {
                auth.Validate();
            }

            foreach (PrivateMessage pm in PrivateMessages)
            {
                pm.Validate();
            }

            foreach (Event ev in Events)
            {
                ev.Validate();
            }
        }

        public int GetAmountNewLikes()
        {
            return Events.Count(x => x.IsRead == false && x.What.Type == "NewLike");
        }

        public List<Event> GetNewLikes()
        {
            return Events.FindAll(x => x.IsRead == false && x.What.Type == "NewLike");
        }

        public int GetAmountNewComments()
        {
            return Events.Count(x => x.IsRead == false && x.What.Type == "NewComment");
        }
        
        public List<Event> GetNewComments()
        {
            return Events.FindAll(x => x.IsRead == false && x.What.Type == "NewComment");
        }

        public int GetAmountNewPrivateMessages()
        {
            return PrivateMessages.Count(x => x.IsRead == false);
        }

        public List<Friend> GetRequestedFriends()
        {
            return Friends.FindAll(x => x.IsRequested.Value && !x.IsApproved.Value && x.IsActive.Value && Login != x.RequestFromLogin);
        }

        public int GetAmountFavourits()
        {
           return Favourits.Count(x => x.Author.IsActive.Value);
        }

        public List<Friend> GetApprovedFriends()
        {
            return Friends.FindAll(x => x.IsApproved.Value && x.IsActive.Value);
        }

        public string GetNameOfNewGroup()
        {
            string baseTemplate = "New Group";

            int idx = 0;

            while(true)
            {
                string nameOfNewGroup;

                if (idx == 0)
                {
                    nameOfNewGroup = baseTemplate;
                }
                else
                {
                    nameOfNewGroup = baseTemplate + " " + idx.ToString();
                }

                if (!Groups.Exists(x => x.Name == nameOfNewGroup ))
                {
                    return nameOfNewGroup;
                }
                
                idx++;
            }
        }

        public string GetNameOfNewLibrary()
        {
            string baseTemplate = "New Library";

            int idx = 0;

            while (true)
            {
                string nameOfNewGroup;

                if (idx == 0)
                {
                    nameOfNewGroup = baseTemplate;
                }
                else
                {
                    nameOfNewGroup = baseTemplate + " " + idx.ToString();
                }

                if (!Libraries.Exists(x => x.Name == nameOfNewGroup))
                {
                    return nameOfNewGroup;
                }

                idx++;
            }
        }
    }
}