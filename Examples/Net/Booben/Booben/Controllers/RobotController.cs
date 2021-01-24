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
    public class RobotController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            string secKey = Request.QueryString["key"];
            string groupName = Request.QueryString["group"];
            string robotName = Request.QueryString["robot"];
            string op = Request.QueryString["op"];
            string retop = Request.QueryString["retop"];

            DAL dal = new DAL();

            try
            {
                string login = dal.GetLogin(secKey);

                ViewBag.Login = login;
                ViewBag.MyLogin = login;
                ViewBag.SecKey = secKey;
                ViewBag.GroupName = groupName;
                ViewBag.RobotName = robotName;

                Robot robot = dal.GetRobot(login, groupName, robotName);

                return View("Info", robot);
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Error");
            }
        }

        private void RequiredContext(Context context)
        {
            Validator.Required(context.SecKey, "SecKey is required for robot");
            Validator.Required(context.Login, "Login is required for robot");
            Validator.Required(context.GroupName, "Group name is required for robot");
            Validator.Required(context.RobotName, "Robot name is required for robot");
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveInfo(Robot robot, string btn)
        {
            Context context = Context.GetContext(Request);

            context.SaveViewBag(ViewBag);

            DAL dal = new DAL();

            try
            {
                //validation
                RequiredContext(context);

                if (btn == "Cancel")
                {
                    return RedirectToAction("Index", "Main", new
                    {
                        op = "editgroup",
                        login = context.Login,
                        group = context.GroupName,
                        key = context.SecKey
                    });
                }
                else
                {
                    Validator.Required(robot.Name, "Robot name is required for robot");

                    //update
                    string login = dal.GetLogin(context.SecKey);

                    dal.UpdateRobot(login, context.GroupName, context.RobotName, robot);

                    context.RobotName = robot.Name;

                    context.SaveViewBag(ViewBag);

                    robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                    return View("Source", robot);
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Info", robot);
            }
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveSource(Robot robot, string btn)
        {
            Context context = Context.GetContext(Request);

            context.SaveViewBag(ViewBag);

            DAL dal = new DAL();

            try
            {
                //validation
                RequiredContext(context);

                //update
                string login = dal.GetLogin(context.SecKey);

                dal.UpdateRobot(login, context.GroupName, context.RobotName, robot);

                if (btn == "Previous")
                {
                    robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                    return View("Info", robot);
                }
                else
                {
                    //validate
                    Validator.Required(robot.Source.Type, "Type is required for source.");

                    foreach (URL url in robot.Source.URLS)
                    {
                        Validator.Required(url.Address, "Source Address is required.");
                    }

                    Validator.Required(robot.Period.Type, "Period Type is required.");

                    //update
                    if (btn == "Add URL")
                    {
                        dal.AddRobotSourceURL(login, context.GroupName, context.RobotName, new URL { IsActive = true });

                        robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                        return View("Source", robot);
                    }
                    else
                    {
                        robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                        return View("Condition", robot);
                    }
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Source", robot);
            }
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveCondition(Robot robot, string btn)
        {
            Context context = Context.GetContext(Request);

            context.SaveViewBag(ViewBag);

            DAL dal = new DAL();

            try
            {
                //validation
                RequiredContext(context);

                //update
                string login = dal.GetLogin(context.SecKey);

                dal.UpdateRobot(login, context.GroupName, context.RobotName, robot);

                robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                if (btn == "Previous")
                {
                    return View("Source", robot);
                }
                else
                {
                    foreach (Condition con in robot.Conditions)
                    {
                        if (con.Library != null)
                        {
                            if (con.Library.Login != null || con.Library.LibName != null)
                            {
                                Validator.Required(con.Library.Login, "Library login is required for condition.");
                                Validator.Required(con.Library.LibName, "Library name is required for condition.");
                            }
                        }
                    }

                    return View("Output", robot);
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Condition", robot);
            }
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveOutput(Robot robot, string btn)
        {
            Context context = Context.GetContext(Request);

            context.SaveViewBag(ViewBag);

            DAL dal = new DAL();

            try
            {
                //update
                string login = dal.GetLogin(context.SecKey);

                robot.LasDocID = 0; //reset

                dal.UpdateRobot(login, context.GroupName, context.RobotName, robot);

                robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                if (btn == "Previous")
                {
                    return View("Condition", robot);
                }
                else
                {
                    Validator.Required(robot.Output.Type, "Output type is required for condition.");

                    return View("Preview", robot);
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Output", robot);
            }
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SavePreview(Robot robot, string btn)
        {
            Context context = Context.GetContext(Request);

            context.SaveViewBag(ViewBag);

            DAL dal = new DAL();

            try
            {
                string login = dal.GetLogin(context.SecKey);

                robot = dal.GetRobot(login, context.GroupName, context.RobotName);

                if (btn == "Previous")
                {
                    return View("Output", robot);
                }
                else
                {
                    Helper.ClearCache(login);

                    return RedirectToAction("Index", "Main", new
                    {
                        op = "editgroup",
                        login = login,
                        group = context.GroupName,
                        key = context.SecKey
                    });
                }

            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;

                return View("Preview", robot);
            }
        }
    }
}
