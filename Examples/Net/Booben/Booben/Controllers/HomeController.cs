using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Text;
using System.IO;

using Booben.Models;
using System.Net;

namespace Booben.Controllers
{
    public class HomeController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            try
            {
                //User user = new Models.User();
                //user.Login = "";
                //user.Author.Likes.Add(new Like() { Login = "bob", IsRead = false, OnDate = DateTime.Now });
                //user.Author.Likes.Add(new Like() { Login = "john", IsRead = false, OnDate = DateTime.Now });

                //Like like = new Like();
                //like.Login = "boo";

                //Author a = new Author { OnDate = DateTime.Now, Likes = { like } };

                //string json = Serialization.SerializeSchema(typeof(User), 16, typeof(Like));

                //string json = Serialization.SerializeSchemaPart(new User { Libraries = { new Library { Author = new Author() } } },
                //                                                typeof(Library));

                //return null;

                //BigDoc DB = new BigDocClient.BigDoc("127.0.0.1", 4477);

                Home home = new Home();

                home.TopItems = new List<Item>();
                home.TopRobots = new Robot[0];

                /*
                if (DB.GetAll().Count() == 0)
                {
                    DB.AddDoc(CreateTestUser());
                }

                //home.TopRobots = GetTopRobots();

                //AddRobot(CreateTestRobot());

                var usrs = DB.GetAll().Select<User>();

                Period period = new Period();
                period.Type = "month";

                foreach (Robot w in home.TopRobots)
                {
                    home.TopItems.AddRange(w.SearchItems(period));
                }
                */

                return View(home);
            }
            catch (Exception ex)
            {
                //ViewBag.Message = ex.Message;
                ViewBag.Message = "Произошла ошибка, приносим свои извинения"; //ex.Message;

                return View("Error");
            }
        }



        /*
         Author Author;

        public string Login { get; set; }
        public string Password { get; set; }
        public string From { get; set; }
        public string Email { get; set; }
        public byte[] Picture { get; set; }
        public int Age { get; set; }
        public string Interests { get; set; }

        Contract Contract;

        public List<Like> Likes { get; set; }
        public List<Group> Groups { get; set; }
        public List<Item> Favourits { get; set; }
        public List<Library> Libraries { get; set; }
        public List<Author> BanList { get; set; }

        public List<Comment> Inbox { get; set; }
        public List<Message> Outbox { get; set; }
        */

        /*
        Author Author { get; set; }

        public string Name { get; set; }
        public string Description { get; set; }
        public byte[] Picture { get; set; }

        public List<Like> Likes { get; set; }
        public List<Message> Messages { get; set; }

        public Period Period { get; set; }
        public int LasDocID { get; set; }

        public Source Source { get; set; }
        public List<Condition> Conditions { get; set; }
        public Output Output { get; set; }

        public List<Notification> Notifications { get; set; }

        public Price Price { get; set; }
        */

        private Robot CreateTestRobot()
        {
            return new Robot()
            {
                Author = new Author()
                {
                    IsPrivate = false,
                    Login = "Bob",
                    OnDate = DateTime.Now,
                    Likes = new List<Like>()
                                    {
                                        new Like()
                                        {
                                            Login = "Mary",
                                            OnDate = DateTime.Now
                                        }
                                    }

                },

                Name = "Robot",
                Description = null,
                Picture = null,

                Period = new Period()
                {
                    FromDate = DateTime.Now,
                    ToDate = DateTime.Now,

                    Type = "Month"
                },

                LasDocID = 10,

                Source = new Source()
                {
                    Type = "All",
                    URLS = null
                },

                Conditions = new List<Condition>()
                                {
                                    new Condition()
                                    {
                                        OnlyTitle = false,
                                        Phrase = "phrase",
                                        Words = null,
                                        MinWords = 1,
                                        UseStemming = true
                                    }
                                },

                Output = new Output()
                {
                    Type = "Text"
                },

                Price = new Price()
                {
                    Value = 10
                }
            };
        }

        private User CreateTestUser()
        {
            User user = new User()
            {
                Author = new Author()
                {
                    IsPrivate = false,
                    Login = "Bob",
                    OnDate = DateTime.Now,
                    Likes = new List<Like>()
                    {
                        new Like()
                        {
                            Login = "Mary",
                            OnDate = DateTime.Now
                        }
                    }
                },

                Login = "Bob",
                Password = "test",
                From = "UA",
                Email = "bob@mail.ru",
                Picture = null,
                Age = 20,
                Interests = "Searching",

                Groups = new List<Group>()
                {
                    new Group()
                    {
                        Name = "Bob Group",
                        Description = null,
                        Author = new Author()
                        {
                            IsPrivate = false,
                            Login = "Bob",
                            OnDate = DateTime.Now,
                            Likes = new List<Like>()
                            {
                                new Like()
                                {
                                    Login = "Mary",
                                    OnDate = DateTime.Now
                                }
                            }
                        },

                        Robots = new List<Robot>()
                        {
                            CreateTestRobot(),
                            CreateTestRobot()
                        }
                    }
                },

                Favourits = new List<Item>()
                {
                    new Item()
                    {
                        Title = "Item title",
                        URL = "Item URL",
                        Text = "Item Text"
                    }
                },

                Libraries = new List<Library>()
                {
                    new Library()
                    {
                        Author = new Author()
                        {
                           IsPrivate = false,
                           Login = "Bob",
                           Likes = new List<Like>()
                           {
                                new Like()
                                {
                                    Login = "Mary",
                                    OnDate = DateTime.Now
                                }
                           },
                           OnDate = DateTime.Now
                        },

                        Category = "Library Category",
                        SubCategory = "Library SubCategory",

                        Name = "Name",
                        Description = null,

                        Words = null
                    }
                },

                BanList = new List<Author>()
                {
                    new Author()
                    {
                        IsPrivate = false,
                        Login = "Bob",
                        OnDate = DateTime.Now
                    }
                }
            };

            return user;

        }
    }
}
