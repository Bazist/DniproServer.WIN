using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.IO;

using Booben.Models;
using System.Net;
using DniproClient;

namespace Booben.Controllers
{
    public class MainController : Controller
    {
        [AcceptVerbs(HttpVerbs.Get)]
        public ActionResult Index()
        {
            string login = Request.QueryString["login"];
            string secKey = Request.QueryString["key"];
            string replyLogin = Request.QueryString["replylogin"];
            string op = Request.QueryString["op"];
            string retop = Request.QueryString["retop"];
            string groupName = Request.QueryString["group"];
            string robotName = Request.QueryString["robot"];
            string libName = Request.QueryString["library"];
            string errorMessage = Request.QueryString["err"];

            Validator.Required(login, "Login is required");
            Validator.Required(secKey, "secKey Key is required");

            DAL dal = new DAL();

            try
            {
                if (op == "markasread")
                {
                    dal.MarkAllAsRead(dal.GetLogin(secKey));

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "marksearchasread")
                {
                    FTSearchService7.FTSearchServiceClient client = new FTSearchService7.FTSearchServiceClient();

                    int lastID = (int)client.GetInfo().LastNameIDRAM;

                    Helper.ClearCache(dal.GetLogin(secKey));

                    dal.MarkSearchAsRead(dal.GetLogin(secKey), groupName, robotName, lastID);

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "refreshsearch")
                {
                    Helper.ClearCache(dal.GetLogin(secKey));

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "setperiod")
                {
                    Helper.ClearCache(dal.GetLogin(secKey));

                    string period = Request["period"];

                    dal.UpdateRobotPeriod(dal.GetLogin(secKey), groupName, robotName, period);

                    op = retop;
                }
                else if (op == "likeuser")
                {
                    dal.LikeUser(login, new Like()
                    {
                        Login = dal.GetLogin(secKey),
                        OnDate = DateTime.Now
                    });

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "likegroup")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.LikeGroup(login, groupName, new Like()
                    {
                        Login = dal.GetLogin(secKey),
                        OnDate = DateTime.Now
                    });

                    op = retop;
                }
                else if (op == "likerobot")
                {
                    Validator.Required(groupName, "Group name is required");
                    Validator.Required(robotName, "Robot name is required");

                    dal.LikeRobot(login, groupName, robotName, new Like()
                    {
                        Login = dal.GetLogin(secKey),
                        OnDate = DateTime.Now
                    });

                    op = retop;
                }
                else if (op == "likelibrary")
                {
                    Validator.Required(groupName, "Library name is required");

                    dal.LikeLibrary(login, libName, new Like()
                    {
                        Login = dal.GetLogin(secKey),
                        OnDate = DateTime.Now
                    });

                    op = retop;
                }
                else if (op == "likecomment")
                {
                    DateTime onDate = DateTime.Parse(Request.QueryString["ondate"]);

                    if (!string.IsNullOrEmpty(libName))
                    {
                        dal.LikeLibraryComment(login, libName, onDate, new Like()
                        {
                            Login = dal.GetLogin(secKey),
                            OnDate = DateTime.Now
                        });

                        op = retop;
                    }
                    else if (!string.IsNullOrEmpty(robotName))
                    {
                        dal.LikeRobotComment(login, groupName, robotName, onDate, new Like()
                        {
                            Login = dal.GetLogin(secKey),
                            OnDate = DateTime.Now
                        });

                        op = retop;
                    }
                    else
                    {
                        throw new Exception("Group name or Library name is required.");
                    }
                }
                else if (op == "addfavourit")
                {
                    int index = Convert.ToInt32(Request["index"]);

                    Robot robot = dal.GetRobot(login, groupName, robotName);

                    List<Item> items = robot.SearchItems(login, groupName, true);
                    Item item = items[index];
                    item.Author.OnDate = DateTime.Now;

                    dal.AddFavourit(dal.GetLogin(secKey), item);

                    op = retop;
                }
                else if (op == "deletefavourit")
                {
                    DateTime onDate = Convert.ToDateTime(Request["ondate"]);

                    dal.DeleteFavourit(dal.GetLogin(secKey), onDate);

                    op = retop;
                }
                else if (op == "addgroup")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.AddGroup(login, new Group { Name = groupName, Author = new Author { Login = login, OnDate = DateTime.Now, IsActive = true, IsPrivate = true } });

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "deletegroup")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.DeleteGroup(login, groupName);

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "addrobot")
                {
                    Validator.Required(groupName, "Group name is required");
                    Validator.Required(robotName, "Robot name is required");

                    dal.AddRobot(login, groupName, new Robot
                    {
                        Name = robotName,
                        Author = new Author { Login = login, OnDate = DateTime.Now, IsActive = true, IsPrivate = true },
                        Source = new Source(),
                        Period = new Period(),
                        Conditions = { new Condition { IsActive = true, MinWords = 1 } },
                        Output = new Output()
                    });

                    op = retop;
                }
                else if (op == "deleterobot")
                {
                    Validator.Required(groupName, "Group name is required");
                    Validator.Required(robotName, "Robot name is required");

                    dal.DeleteRobot(login, groupName, robotName);

                    op = retop;
                }
                else if (op == "addnotification")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.AddNotification(dal.GetLogin(secKey), groupName, new Notification { IsActive = true, OnDate = DateTime.Now });

                    op = retop;
                }
                else if (op == "deletenotification")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.DeleteNotification(dal.GetLogin(secKey), groupName, DateTime.Parse(Request["ondate"]));

                    op = retop;
                }
                else if (op == "copygroup")
                {
                    Validator.Required(groupName, "Group name is required");

                    string toLogin = dal.GetLogin(secKey);

                    dal.CopyGroup(login, toLogin, groupName);

                    op = retop;
                }
                else if (op == "subscribegroup")
                {
                    Validator.Required(groupName, "Group name is required");

                    Subscriber subscriber = new Subscriber { Login = dal.GetLogin(secKey), IsActive = true };

                    dal.AddSubscriberToRobot(login, groupName, robotName, subscriber);

                    op = retop;
                }
                else if (op == "unsubscribegroup")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.DeleteSubscriberFromRobot(login, groupName, robotName, dal.GetLogin(secKey));

                    op = retop;
                }
                else if (op == "unsubscriberobot")
                {
                    Validator.Required(groupName, "Group name is required");

                    dal.DeleteSubscriberFromRobot(login, groupName, robotName, dal.GetLogin(secKey));

                    op = retop;
                }
                else if (op == "subscribelibrary")
                {
                    Validator.Required(libName, "Library name is required");

                    Subscriber subscriber = new Subscriber { Login = dal.GetLogin(secKey), IsActive = true };

                    dal.AddSubscriberToLibrary(login, libName, subscriber);

                    op = retop;
                }
                else if (op == "unsubscribelibrary")
                {
                    Validator.Required(libName, "Library name is required");

                    dal.DeleteSubscriberFromLibrary(login, libName, dal.GetLogin(secKey));

                    op = retop;
                }
                else if (op == "privmessages")
                {
                    dal.MarkReadPrivateMessages(dal.GetLogin(secKey));
                }
                else if (op == "addlibrary")
                {
                    Validator.Required(libName, "Library name is required");

                    dal.AddLibrary(login, new Library
                    {
                        Category = "Base",
                        SubCategory = "Sub Base",
                        Name = libName,
                        Author = new Author
                        {
                            Login = login,
                            OnDate = DateTime.Now,
                            IsActive = true
                        }
                    });

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "deletelibrary")
                {
                    Validator.Required(libName, "Library name is required");

                    dal.DeleteLibrary(login, libName);

                    return RedirectToAction("Index", "Main", new { login = login, key = secKey });
                }
                else if (op == "requestfriend")
                {
                    string friendLogin = Request["friendlogin"];

                    dal.AddFriend(dal.GetLogin(secKey), new Friend
                    {
                        IsActive = true,
                        IsApproved = false,
                        IsRequested = true,
                        OnDate = DateTime.Now,
                        RequestFromLogin = dal.GetLogin(secKey),
                        Login = friendLogin
                    });

                    dal.AddFriend(friendLogin, new Friend
                    {
                        IsActive = true,
                        IsApproved = false,
                        IsRequested = true,
                        OnDate = DateTime.Now,
                        RequestFromLogin = dal.GetLogin(secKey),
                        Login = dal.GetLogin(secKey)
                    });

                    op = retop;
                }
                else if (op == "approvefriend")
                {
                    string friendLogin = Request["friendlogin"];

                    dal.ApproveFriend(dal.GetLogin(secKey), friendLogin);

                    op = retop;
                }
                else if (op == "deletefriend")
                {
                    string friendLogin = Request["friendlogin"];

                    dal.DeleteFriend(dal.GetLogin(secKey), friendLogin);
                    dal.DeleteFriend(friendLogin, dal.GetLogin(secKey));

                    op = retop;
                }
            }
            catch (Exception ex)
            {
                ViewBag.ErrorMessage = ex.Message;
            }

            User user = dal.GetUserByLogin(login); //full load
            string myLogin = dal.GetLogin(secKey);

            if (user.Login == myLogin)
            {
                ViewBag.IsMyGroup = true;
            }
            else
            {
                ViewBag.IsMyGroup = false;
            }

            ViewBag.ReturnOperation = op;
            ViewBag.Login = user.Login;
            ViewBag.ReplyLogin = replyLogin;
            ViewBag.MyLogin = myLogin;
            ViewBag.SecKey = secKey;
            ViewBag.GroupName = groupName;
            ViewBag.RobotName = robotName;
            ViewBag.LibName = libName;
            ViewBag.ErrorMessage = errorMessage;

            if (!string.IsNullOrEmpty(groupName))
            {
                ViewBag.IndexGroup = user.Groups.FindIndex(x => x.Name == groupName);
            }

            if (!string.IsNullOrEmpty(robotName))
            {
                ViewBag.IndexRobot = user.Groups.Find(x => x.Name == groupName).Robots.FindIndex(x => x.Name == robotName);
            }

            if (!string.IsNullOrEmpty(libName))
            {
                ViewBag.IndexLibrary = user.Libraries.FindIndex(x => x.Name == libName);
            }

            ViewBag.Operation = op;

            return View(user);

        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveUser(User user, string btn)
        {
            Context context = Context.GetContext(Request);

            DAL dal = new DAL();

            try
            {
                if (btn == "Save")
                {
                    Validator.Required(user.Login, "Login is required.");
                    Validator.Required(user.Email, "Email is required.");

                    dal.UpdateUser(user, Change.CreateOne(user, ChangeEnum.UpdateOnlyTopLevel));
                }

                return RedirectToAction("Index", "Main", new { login = user.Login, key = user.SecKey });
            }
            catch (Exception ex)
            {
                return RedirectToAction("Index", "Main", new { op = "edituser", err = ex.Message, login = context.Login, key = context.SecKey });
            }
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveGroup(Group group, string btn)
        {
            Context context = Context.GetContext(Request);

            DAL dal = new DAL();
            try
            {
                if (context.Operation == "editgroup")
                {
                    string myLogin = dal.GetLogin(context.SecKey);

                    if (!string.IsNullOrEmpty(myLogin))
                    {
                        Validator.Required(group.Name, "Group name is required.");

                        dal.UpdateGroup(context.MyLogin, context.GroupName, group);

                        return RedirectToAction("Index", "Main", new { op = "group", group = group.Name, login = context.Login, key = context.SecKey });
                    }
                }

            }
            catch (Exception ex)
            {
                return RedirectToAction("Index", "Main", new { op = "group", err = ex.Message, group = group.Name, login = context.Login, key = context.SecKey });
            }

            return RedirectToAction("Index", "Main"); //, new { login = user.Login, key = user.SecKey });
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SaveLibrary(Library lib, string btn)
        {
            Context context = Context.GetContext(Request);

            DAL dal = new DAL();

            if (context.Operation == "editlibrary")
            {
                string myLogin = dal.GetLogin(context.SecKey);

                if (!string.IsNullOrEmpty(myLogin))
                {
                    try
                    {
                        Validator.Required(lib.Category, "Category is required.");
                        Validator.Required(lib.SubCategory, "Sub Category name is required.");
                        Validator.Required(lib.Name, "Library name is required.");

                        dal.UpdateLibrary(context.MyLogin, context.LibName, lib);

                        return RedirectToAction("Index", "Main", new { op = "library", library = lib.Name, login = context.Login, key = context.SecKey });
                    }
                    catch (Exception ex)
                    {
                        return RedirectToAction("Index", "Main", new { op = "library", err = ex.Message, library = lib.Name, login = context.Login, key = context.SecKey });
                    }
                }
            }

            return RedirectToAction("Index", "Main"); //, new { login = user.Login, key = user.SecKey });
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SendComment(string comment)
        {
            Context context = Context.GetContext(Request);

            DAL dal = new DAL();

            string myLogin = dal.GetLogin(context.SecKey);

            if (context.Operation == "group")
            {
                if (!string.IsNullOrEmpty(myLogin))
                {
                    try
                    {
                        Validator.Required(context.GroupName, "Group name is required");
                        Validator.Required(comment, "Comment is required.");

                        Comment comm = new Comment();
                        comm.Author = new Author { Login = myLogin, OnDate = DateTime.Now };
                        comm.Text = comment;

                        dal.AddCommentToRobot(context.Login, context.GroupName, context.RobotName, comm);

                        return RedirectToAction("Index", "Main", new { op = "group", group = context.GroupName, login = context.Login, key = context.SecKey });
                    }
                    catch (Exception ex)
                    {
                        return RedirectToAction("Index", "Main", new { op = "group", err = ex.Message, group = context.GroupName, login = context.Login, key = context.SecKey });
                    }
                }
            }
            else if (context.Operation == "library")
            {
                if (!string.IsNullOrEmpty(myLogin))
                {
                    try
                    {
                        Validator.Required(context.LibName, "Library name is required");
                        Validator.Required(comment, "Comment is required.");

                        Comment comm = new Comment();
                        comm.Author = new Author { Login = myLogin, OnDate = DateTime.Now };
                        comm.Text = comment;

                        dal.AddCommentToLibrary(context.Login, context.LibName, comm);

                        return RedirectToAction("Index", "Main", new { op = "library", library = context.LibName, login = context.Login, key = context.SecKey });
                    }
                    catch (Exception ex)
                    {
                        return RedirectToAction("Index", "Main", new { op = "library", err = ex.Message, library = context.LibName, login = context.Login, key = context.SecKey });
                    }
                }
            }

            return RedirectToAction("Index", "Main");
        }

        [HttpPost, ValidateInput(false)]
        public ActionResult SendPrivateMessage(string toLogin, string message)
        {
            Context context = Context.GetContext(Request);

            DAL dal = new DAL();

            try
            {
                Validator.Required(toLogin, "To login is required");
                Validator.Required(toLogin, "Message is required");

                string myLogin = dal.GetLogin(context.SecKey);

                dal.AddPrivateMessage(myLogin, new PrivateMessage { OnDate = DateTime.Now, FromLogin = myLogin, ToLogin = toLogin, IsRead = true, Message = message });
                dal.AddPrivateMessage(toLogin, new PrivateMessage { OnDate = DateTime.Now, FromLogin = myLogin, ToLogin = toLogin, IsRead = false, Message = message });

                return RedirectToAction("Index", "Main", new { op = "privmessages", login = context.Login, key = context.SecKey });
            }
            catch (Exception ex)
            {
                return RedirectToAction("Index", "Main", new { op = "privmessages", err = ex.Message, login = context.Login, key = context.SecKey });
            }
        }
    }
}
