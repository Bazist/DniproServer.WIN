using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Text;
using System.Text.RegularExpressions;

using PikoNet.Models;
using PikoNet.FTSearchService;
using DniproClient;

namespace PikoNet.Controllers
{
    public class HomeController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            Article[] a = DAL._db.GetAll().Sort("{'UpdatedDate':$}", false)
                                     .Select<Article>("{'ID':$,'Author':$,'LastAuthor':$,'Title':$,'CountMessage':$,'UpdatedDate':$}");

            return View(a);
        }

        public ActionResult send(string btn)
        {
            return View("NewArticle", new Article());
        }

    }
}
