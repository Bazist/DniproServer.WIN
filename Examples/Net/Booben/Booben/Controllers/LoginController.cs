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
    public class LoginController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            User user = new Models.User();

            user.Login = "aaa";
            user.Password = "1";

            return View(user);
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult Login(User user)
        {
            DAL dal = new DAL();

            try
            {

                //dal.Index();

                Validator.Required(user.Login, "Login is required.");
                Validator.Required(user.Password, "Password is required.");

                string secKey = dal.GetSecKey(user.Login, user.Password);

                if (secKey != null)
                {
                    Helper.ClearCache(user.Login);

                    return RedirectToAction("Index", "Main", new { login = user.Login, key = secKey });
                }
                else
                {
                    ViewBag.ErrorMessage = "Login or Password is wrong.";

                    return View("Index", user);
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Index");
            }
        }
    }
}
