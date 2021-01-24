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
    public class NewMessageController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index(int id)
        {
            Message m = new Message();
            m.ArticleID = id;

            return View(m);
        }

        public ActionResult Send(Message m)
        {
            m.Validate();

            m.CreatedDate = DateTime.Now.Ticks;

            var countMessage = DAL.GetCountMessage(m.ArticleID);

            DAL._db.GetWhere("{'ID':" + m.ArticleID.ToString() + "}")
              .Insert<Message>(m, "{'CountMessage':" + (countMessage + 1).ToString() + ",'LastAuthor':'" + m.Author + "','Messages':[Add,!{'Author':$,'Text':$,'CreatedDate':$}]}");

            return RedirectToAction("Index", "Article", new { id = m.ArticleID, cp = (countMessage / 3 + 1) });
        }
    }
}
