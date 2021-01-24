using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;

using Booben.Models;
using System.Net;

namespace Booben.Controllers
{
    public class SearchController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            try
            { 
                return View();
            }
            catch (Exception ex)
            {
                //ViewBag.Message = ex.Message;
                ViewBag.Message = "Произошла ошибка, приносим свои извинения"; //ex.Message;

                return View("Error");
            }
        }
    }
}
