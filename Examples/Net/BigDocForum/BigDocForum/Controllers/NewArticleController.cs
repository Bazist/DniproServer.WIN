using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Text;
using System.Text.RegularExpressions;

using BigDocForum.Models;
using BigDocForum.FTSearchService;
using BigDocClient;

namespace BigDocForum.Controllers
{
    public class NewArticleController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            return View();
        }

        //var a = new Article()
        //{
        //    ID = Guid.NewGuid(),
        //    Author = "Ivanov",
        //    Title = "My first artcile",
        //    CreatedDate = DateTime.Now.Ticks,
        //    UpdatedDate = DateTime.Now.Ticks,
        //    CountMessage = 1,
        //    Text = "Hello world",

        //    Messages = new List<Message>()
        //    {
        //        new Message()
        //        {
        //            Author = "Ivanov",
        //            CreatedDate = DateTime.Now.Ticks,
        //            Text = "First answer",
        //        }
        //    }
        //};

        //db.AddDoc(a);

        public ActionResult send(Article a)
        {
            a.Validate();

            a.ID = DAL._db.GetAll().Count() + 1;

            a.CreatedDate = DateTime.Now.Ticks;
            a.UpdatedDate = DateTime.Now.Ticks;

            DAL._db.AddDoc(a);

            return RedirectToAction("Index", "Home");
        }

    }
}
