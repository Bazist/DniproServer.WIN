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
using DniproClient;

namespace Booben.Controllers
{
    public class RegistrationController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            DAL dal = new DAL();

            User user = dal.CreateDefaultUser();

            dal.AddUser(user);

            return View("Step1", user);
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult Next(User user)
        {
            DAL dal = new DAL();

            try
            {
                dal.UpdateUser(user, Change.CreateOne(user.Groups));

                user = dal.GetUser(user.SecKey);

                return View("Step2", user);
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Step2", dal.GetUser(user.SecKey));
            }
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult Register(User user, string btn)
        {
            DAL dal = new DAL();

            try
            {
                if (btn.Contains("Register"))
                {
                    if (!dal.IsLoginExists(user.Login))
                    {
                        Validator.Required(user.Login, "Login is required.");
                        Validator.Required(user.Password, "Password is required.");
                        Validator.Required(user.Email, "Email is required.");

                        if(user.Password != Request["RepeatPassword"])
                        {
                            throw new Exception("Passwords are not equals.");
                        }

                        dal.UpdateUser(user, Change.CreateOne(user, ChangeEnum.UpdateOnlyTopLevel));

                        return RedirectToAction("Index", "Main", new { login = user.Login, key = user.SecKey });
                    }
                    else
                    {
                        throw new Exception("User is exists.");
                    }
                }
                else //back
                {
                    dal.UpdateUser(user, Change.CreateOne(user, ChangeEnum.UpdateOnlyTopLevel));

                    return View("Step1", dal.GetUser(user.SecKey));
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Step2", dal.GetUser(user.SecKey));
            }
        }
    }
}
