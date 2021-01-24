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
    public class ArticleController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index(int id)
        {
            var page = int.Parse(Request.QueryString["cp"] ?? "1");

            Article[] a = DAL._db.GetWhere("{'ID':" + id +"}")
                                 .Select<Article>("{'Author':$,'Title':$,'CountMessage':$,'CreatedDate':$,'ID':$,'Text':$," +
                                                                           "'Messages':[P," + ((page - 1) * 3 + 1) + ",3,{'ArticleID':$,'Author':$,'CreatedDate':$,'Text':$}]}");

            a[0].CurrentPage = page;

            return View(a[0]);
        }

        public ActionResult send(Article a)
        {
            return RedirectToAction("Index", "NewMessage", new { id = a.ID });
        }

    }
}
