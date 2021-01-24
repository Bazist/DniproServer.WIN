using Booben.Models;
using DniproClient;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Web;

namespace Booben
{
    public class DAL
    {
        public static DniproDB DB = new DniproDB("127.0.0.1", 4477);

        private void SaveUserFileLog(string login)
        {
            if (!string.IsNullOrEmpty(login))
            {
                User user = GetUserByLogin(login);

                string json = Serialization.FormatJson(Serialization.SerializeChanges(user));

                File.WriteAllText(@"C:\FTS\DniproExamples\Booben\Booben\Users\" + login + ".txt", json);
            }
        }

        public User CreateDefaultUser()
        {
            User user = new User();
            user.Author = new Author();
            user.Author.IsActive = true;
            user.SecKey = Guid.NewGuid().ToString("N");
            user.Groups.Add(new Group() { Name = "Media",
                                          Author = new Author { IsActive = true },
                                          Robots = { new Robot { Author = new Author { IsActive = true },
                                                                 Name = "Coub",
                                                                 Source = new Source { Type = "All" },
                                                                 Conditions = { new Condition { Phrase = "" } },
                                                                 Output = new Output {Type = "coub" } },
                                                     new Robot { Author = new Author { IsActive = true },
                                                                 Name = "Gif",
                                                                 Source = new Source { Type = "All" },
                                                                 Conditions = { new Condition { Phrase = "" } },
                                                                 Output = new Output {Type = "gif" } },
                                                     new Robot { Author = new Author { IsActive = true },
                                                                 Name = "Video",
                                                                 Source = new Source { Type = "All" },
                                                                 Conditions = { new Condition { Phrase = "" } },
                                                                 Output = new Output {Type = "video" } } } });
            
            return user;
        }

        public void AddUser(User user)
        {
            user.Validate();

            DB.AddDoc(user);
        }

        public void UpdateUser(User user, Change[] changes = null)
        {
            user.Validate();

            DB.GetWhere<User>(new User { SecKey = user.SecKey })
              .Update<User>(user, null, changes);

            this.SaveUserFileLog(user.Login);
        }

        public User GetUser(string secKey)
        {
            return DB.GetWhere<User>(new User { SecKey = secKey }).Select<User>()[0];
        }

        public User LoadUser(User user, Load[] loads)
        {
            return DB.GetWhere<User>(new User { Login = user.Login })
                     .Select<User>(user, null, loads);
        }

        public User GetUserByLogin(string login, uint maxDepth = Serialization.MaxDepth)
        {
            return DB.GetWhere<User>(new User { Login = login })
                     .Select<User>(null, maxDepth)[0];
        }

        public Group GetGroup(string login, string groupName, uint maxDepth = Serialization.MaxDepth)
        {
            return DB.GetWhere<User>(new User { Login = login, Groups = { new Group { Name = groupName } } })
                     .Select<Group>(new User { Groups = { new Group() } });
        }

        public Robot GetRobot(string login, string groupName, string widName, uint maxDepth = Serialization.MaxDepth)
        {
            return DB.GetWhere<User>(new User { Login = login, Groups = { new Group { Name = groupName, Robots = { new Robot { Name = widName } } } } })
                     .Select<Robot>(new User { Groups = { new Group { Robots = { new Robot() } } } });
        }

        public Library GetLibrary(string login, string libName, uint maxDepth = Serialization.MaxDepth)
        {
            return DB.GetWhere<User>(new User { Login = login, Libraries = { new Library { Name = libName } } })
                     .Select<Library>(new User { Libraries = { new Library() } });
        }

        public void Index()
        {
            DB.AddDoc(File.ReadAllText(@"C:\FTS\DniproExamples\Booben\Booben\Users\aaa.txt"));
        }

        public string GetSecKey(string login, string password)
        {
            return DB.GetWhere<User>(new User { Login = login, Password = password })
                     .Value(new User { SecKey = "" }); //lambda
        }

        public string GetLogin(string secKey)
        {

            return DB.GetWhere(new User { SecKey = secKey })
                     .Value(new User { Login = "" });
        }

        public bool IsUserExists(string secKey)
        {
            return DB.GetWhere(new User { SecKey = secKey }).Count() > 0;
        }

        public bool IsLoginExists(string login)
        {
            return DB.GetWhere(new User { Login = login }).Count() > 0;
        }

        public Robot[] GetTopRobots()
        {
            return new Robot[0];
        }

        public void AddGroup(string login, Group group)
        {
            group.Validate();

            DB.GetWhere(new User { Login = login })
              .Insert(new User { Groups = { group } },
                      Change.CreateOne(group, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void AddLibrary(string login, Library lib)
        {
            lib.Validate();

            DB.GetWhere(new User { Login = login })
              .Insert(new User { Libraries = { lib } },
                      Change.CreateOne(lib, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void AddRobot(string login, string groupName, Robot robot)
        {
            robot.Validate();

            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName } } })
              .Insert(new User { Groups = { new Group { Robots = { robot } } } },
                      Change.CreateOne(robot, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void AddRobotSourceURL(string login, string groupName, string widName, URL url)
        {
            url.Validate();

            DB.GetWhere(new User { Login = login,
                Groups = { new Group { Name = groupName,
                                                             Robots = { new Robot { Name = widName } } } } })
              .Insert(new User { Groups = { new Group { Robots = { new Robot { Source = new Source { URLS = { url } } } } } } },
                      Change.CreateOne(url, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void AddNotification(string login, string groupName, Notification notification)
        {
            notification.Validate();

            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName } } })
              .Insert(new User { Groups = { new Group { Notifications = { notification } } } },
                      Change.CreateOne(notification, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void AddFavourit(string login, Item item)
        {
            //item.Validate();
            item.Title = Serialization.Encode(item.Title);
            item.Text = Serialization.Encode(item.Text);
           
            DB.GetWhere(new User { Login = login })
              .Insert(new User { Favourits = { item } },
                      Change.CreateOne(item, ChangeEnum.Add));

            //.Insert(x => x.Favourits, item)

            this.SaveUserFileLog(login);
        }

        public void DeleteFavourit(string login, DateTime onDate)
        {
            //item.Validate();
            DB.GetWhere(new User { Login = login, Favourits = { new Item { Author = new Author { OnDate = onDate } } } })
              .Update(new User { Favourits = { new Item { Author = new Author { IsActive = false } } } });

            this.SaveUserFileLog(login);
        }

        public void AddPrivateMessage(string login, PrivateMessage privMessage)
        {
            privMessage.Validate();

            DB.GetWhere(new User { Login = login })
                .Insert(new User { PrivateMessages = { privMessage } },
                        Change.CreateOne(privMessage, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void MarkReadPrivateMessages(string login)
        {
            DB.GetWhere(new User { Login = login, PrivateMessages = { new PrivateMessage { IsRead = false } } })
              .Update(new User { PrivateMessages = { new PrivateMessage { IsRead = true } } });

            this.SaveUserFileLog(login);
        }

        public void UpdateGroup(string login, string groupName, Group group)
        {
            group.Validate();

            DB.GetWhere<User>(new User { Login = login, Groups = { new Group { Name = groupName } } })
              .Update<User>(new User { Groups = { group } });

            this.SaveUserFileLog(login);
        }

        public void UpdateRobot(string login, string groupName, string widName, Robot robot)
        {
            robot.Validate();

            DB.GetWhere<User>(new User { Login = login, Groups = { new Group { Name = groupName, Robots = { new Robot { Name = widName } } } } })
              .Update<User>(new User { Groups = { new Group { Robots = { robot } } } });

            this.SaveUserFileLog(login);
        }

        public void UpdateLibrary(string login, string libName, Library lib)
        {
            lib.Validate();

            DB.GetWhere<User>(new User { Login = login, Libraries = { new Library { Name = libName } } })
              .Update<User>(new User { Libraries = { lib } });

            this.SaveUserFileLog(login);
        }

        public void CopyGroup(string fromLogin, string toLogin, string groupName)
        {
            Group group =
                DB.GetWhere(new User { Login = fromLogin, Groups = { new Group { Name = groupName } } })
                    .Select<Group>(new User { Groups = { new Group() } });

            AddGroup(toLogin, group);

            AddEvent(Event.Create(fromLogin, "Group", fromLogin, groupName, null, null, DateTime.MinValue,
                                  "CopyGroup", toLogin, string.Format("Your group {0} copied by {1}.", groupName, toLogin)));
        }

        public void DeleteGroup(string login, string groupName)
        {
            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName } } })
              .Update<User>(new User { Groups = { new Group { Author = new Author { IsActive = false } } } });

            this.SaveUserFileLog(login);
        }

        public void DeleteLibrary(string login, string libName)
        {
            DB.GetWhere(new User { Login = login, Libraries = { new Library { Name = libName } } })
              .Update<User>(new User { Libraries = { new Library { Author = new Author { IsActive = false } } } });

            this.SaveUserFileLog(login);
        }

        public void DeleteRobot(string login, string groupName, string robotName)
        {
            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName, Robots = { new Robot { Name = robotName } } } } })
              .Update<User>(new User { Groups = { new Group { Robots = { new Robot { Author = new Author { IsActive = false } } } } } });

            this.SaveUserFileLog(login);
        }

        public void DeleteNotification(string login, string groupName, DateTime onDate)
        {
            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName, Notifications = { new Notification { OnDate = onDate } } } } })
              .Update<User>(new User { Groups = { new Group { Notifications = { new Notification { IsActive = false } } } } });

            this.SaveUserFileLog(login);
        }

        public void CopyRobot(string fromLogin, string fromGroupName, string toLogin, string toGroupName, string robotName)
        {
            Robot[] robots =
                DB.GetWhere("{'Login':'" + fromLogin + "','Groups':[{'Name':'" + fromGroupName + "','Robots':[{'Name':'" + robotName + "'}]}]}")
                    .Select<Robot>("{'Groups':[{'Robots':[!" + Serialization.SerializeSchema(typeof(Robot)) + "]}]}");

            AddRobot(toLogin, toGroupName, robots[0]);

            AddEvent(Event.Create(fromLogin, "Robot", fromLogin, fromGroupName, robotName, null, DateTime.MinValue,
                                  "CopyGroup", toLogin, string.Format("Your robot {0} copied by {1}.", fromGroupName, toLogin)));
        }

        #region Likes

        public void LikeUser(string login, Like like)
        {
            like.Validate();

            DB.GetWhere(new User { Login = login })
                .Insert(new User { Author = new Author { Likes = { like } } },
                        Change.CreateOne(like, ChangeEnum.Add));

            AddEvent(Event.Create(login, "User", login, null, null, null, DateTime.MinValue,
                                  "NewLike", like.Login, string.Format("Your profile has new like from {0}.", like.Login)));

            this.SaveUserFileLog(login);
        }

        public void LikeGroup(string login, string groupName, Like like)
        {
            like.Validate();

            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName } } })
                .Insert(new User { Groups = { new Group { Author = new Author { Likes = { like } } } } },
                        Change.CreateOne(like, ChangeEnum.Add));

            AddEvent(Event.Create(login, "Group", login, groupName, null, null, DateTime.MinValue,
                                  "NewLike", like.Login, string.Format("Your group {0} has new like from {1}.", groupName, like.Login)));

            this.SaveUserFileLog(login);
        }

        public void LikeRobot(string login, string groupName, string robotName, Like like)
        {
            like.Validate();

            DB.GetWhere(new User { Login = login, Groups = { new Group { Name = groupName, Robots = { new Robot { Name = robotName } } } } })
                .Insert(new User { Groups = { new Group { Robots = { new Robot { Author = new Author { Likes = { like } } } } } } },
                        Change.CreateOne(like));

            AddEvent(Event.Create(login, "Robot", login, groupName, robotName, null, DateTime.MinValue,
                                  "NewLike", like.Login, string.Format("Your robot {0} has new like from {1}.", robotName, like.Login)));

            this.SaveUserFileLog(login);
        }

        public void LikeLibrary(string login, string libName, Like like)
        {
            like.Validate();

            DB.GetWhere(new User { Login = login, Libraries = { new Library { Name = libName } } })
              .Insert(new User { Libraries = { new Library { Author = new Author { Likes = { like } } } } },
                      Change.CreateOne(like, ChangeEnum.Add));

            AddEvent(Event.Create(login, "Library", login, null, null, libName, DateTime.MinValue,
                                  "NewLike", like.Login, string.Format("Your library {0} has new like from {1}.", libName, like.Login)));

            this.SaveUserFileLog(login);
        }

        #region Comment

        public void LikeRobotComment(string login, string groupName, string robotName, DateTime commentDate, Like like)
        {
            like.Validate();

            DB.GetWhere(new User
            {
                Login = login,
                Groups = { new Group { Name = groupName,
                                       Robots = { new Robot { Name = robotName,
                                                              Comments = { new Comment { Author = new Author { OnDate = commentDate } } } } } } }
            })
            .Insert(new User { Groups = { new Group { Robots = { new Robot { Comments = { new Comment { Author = new Author { Likes = { like } } } } } } } } },
                    Change.CreateOne(like, ChangeEnum.Add));


            AddEvent(Event.Create(login, "Comment", login, groupName, robotName, null, commentDate,
                                  "NewLike", like.Login, string.Format("Your comment has new like from {0}.", like.Login)));

            this.SaveUserFileLog(login);
        }

        public void LikeLibraryComment(string login, string libName, DateTime commentDate, Like like)
        {
            like.Validate();

            DB.GetWhere(new User
            {
                Login = login,
                Libraries = { new Library { Name = libName,
                                            Comments = {new Comment {Author = new Author {OnDate = commentDate}}}}}
            })
            .Insert(new User
            {
                Libraries = { new Library { Comments = { new Comment { Author = new Author { Likes = { like } } } } } }
            },
            Change.CreateOne(like, ChangeEnum.Add));

            AddEvent(Event.Create(login, "Comment", login, null, null, libName, commentDate,
                                  "NewLike", like.Login, string.Format("Your library {0} has new like from {1}.", libName, like.Login)));

            this.SaveUserFileLog(login);
        }

        public Comment GetRobotComment(string login, string groupName, string robotName, DateTime? commentDate)
        {
            return DB.GetWhere(new User
            {
                Login = login,
                Groups = { new Group { Name = groupName,
                                       Robots = { new Robot { Name = robotName,
                                                              Comments = { new Comment { Author = new Author { OnDate = commentDate } } } } } } }
            })
            .Select<Comment>(new User { Groups = { new Group { Robots = { new Robot { Comments = { new Comment() } } } } } });
        }

        public Comment GetLibraryComment(string login, string libName, DateTime? commentDate)
        {
            return DB.GetWhere(new User
            {
                Login = login,
                Libraries = { new Library { Name = libName,
                                            Comments = {new Comment {Author = new Author {OnDate = commentDate}}}}}
            })
            .Select<Comment>(new User { Libraries = { new Library { Comments = { new Comment() } } } });
        }

        #endregion

        #endregion

        #region Comments

        public void AddCommentToRobot(string login, string groupName, string robotName, Comment comment)
        {
            comment.Validate();

            //inert comment
            DB.GetWhere(new User { Login = login,
                                   Groups = { new Group { Name = groupName,
                                   Robots = { new Robot { Name = robotName } } } } })
                .Insert(new User { Groups = { new Group { Robots = { new Robot { Comments = { comment } } } } } },
                        Change.CreateOne(comment, ChangeEnum.Add));

            //create event
            Event ev = Event.Create(login, "Group", login, groupName, robotName, null, DateTime.MinValue,
                                    "NewComment", comment.Author.Login, string.Format("Your group {0} has new comment from {1}.", groupName, comment.Author.Login));

            //notify subscribers
            DB.GetWhere(new User
            {
                Login = login,
                Groups = { new Group { Name = groupName,
                                       Robots = {new Robot { Name = robotName,
                                                             Subscribers = { new Subscriber { IsActive = true } } } } } }
            })
            .Join(new User { Groups = { new Group { Robots = { new Robot { Subscribers = { new Subscriber { Login = "" } } } } } } }, //lambda
                  new User { Login = "" })
            .Insert(new User { Events = { ev } },
                    Change.CreateOne(ev, ChangeEnum.Add),
                    JoinedEnum.SecondTable);

            if (login != comment.Author.Login) //do not send to yourself
            {
                //notify me
                AddEvent(ev);
            }

            this.SaveUserFileLog(login);
        }

        public void AddCommentToLibrary(string login, string libName, Comment comment)
        {
            comment.Validate();

            //insert comment
            DB.GetWhere(new User { Login = login, Libraries = { new Library { Name = libName } } })
                .Insert(new User { Libraries = { new Library { Comments = { comment } } } },
                        Change.CreateOne(comment, ChangeEnum.Add));

            //create event
            Event ev = Event.Create(login, "Library", login, null, null, libName, DateTime.MinValue,
                                    "NewComment", comment.Author.Login, string.Format("Your library {0} has new comment from {1}.", libName, comment.Author.Login));

            //notify subscribers
            DB.GetWhere(new User { Login = login, Libraries = { new Library { Name = libName, Subscribers = { new Subscriber { IsActive = true } } } } })
                .Join(new User { Libraries = { new Library { Subscribers = { new Subscriber { Login = "" } } } } },
                      new User { Login = "" })
                .Insert(new User { Events = { ev } },
                        Change.CreateOne(ev, ChangeEnum.Add),
                        JoinedEnum.SecondTable);

            if (login != comment.Author.Login) //do not send to yourself
            {
                //notify me
                AddEvent(ev);
            }

            this.SaveUserFileLog(login);
        }

        #endregion

        #region Subscribers

        public void AddSubscriberToRobot(string login, string groupName, string robotName, Subscriber subscriber)
        {
            subscriber.Validate();

            DB.GetWhere(new User { Login = login,
                Groups = { new Group { Name = groupName,
                                                          Robots = { new Robot { Name = robotName } } } } })
                .Insert(new User { Groups = { new Group { Robots = { new Robot { Subscribers = { subscriber } } } } } },
                                   Change.CreateOne(subscriber, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void DeleteSubscriberFromRobot(string login, string groupName, string robotName, string myLogin)
        {
            DB.GetWhere(new User
            {
                Login = login,
                Groups = { new Group { Name = groupName,
                                       Robots = {new Robot { Name = robotName,
                                                             Subscribers = { new Subscriber { Login = myLogin } } } } } }
            }).Update(new User { Groups = { new Group { Robots = { new Robot { Subscribers = { new Subscriber { IsActive = false } } } } } } });

            this.SaveUserFileLog(login);
        }

        public void AddSubscriberToLibrary(string login, string libName, Subscriber subscriber)
        {
            subscriber.Validate();

            DB.GetWhere(new User { Login = login, Libraries = { new Library { Name = libName } } })
                .Insert(new User { Libraries = { new Library { Subscribers = { subscriber } } } },
                        Change.CreateOne(subscriber, ChangeEnum.Add));
                        
            this.SaveUserFileLog(login);
        }

        public void DeleteSubscriberFromLibrary(string login, string libName, string myLogin)
        {
            DB.GetWhere(new User
            {
                Login = login,
                Libraries = { new Library { Name = libName,
                                            Subscribers = { new Subscriber { Login = myLogin } } } }
            }).Update(new User { Libraries = { new Library { Subscribers = { new Subscriber { IsActive = false } } } } });

            this.SaveUserFileLog(login);
        }

        #endregion

        #region Friends

        public void AddFriend(string login, Friend friend)
        {
            DB.GetWhere(new User { Login = login })
              .Insert(new User { Friends = { friend } },
                      Change.CreateOne(friend, ChangeEnum.Add));

            this.SaveUserFileLog(login);
        }

        public void ApproveFriend(string login, string friendLogin)
        {
            DB.GetWhere(new User { Login = login, Friends = { new Friend { Login = friendLogin } } })
              .Update(new User { Friends = { new Friend { IsApproved = true } } });

            this.SaveUserFileLog(login);
        }

        public void DeleteFriend(string login, string friendLogin)
        {
            DB.GetWhere(new User { Login = login, Friends = { new Friend { Login = friendLogin } } })
              .Update(new User { Friends = { new Friend { IsActive = false } } });

            this.SaveUserFileLog(login);
        }

        #endregion

        public void MarkAllAsRead(string login)
        {
            DB.GetWhere(new User { Login = login, Events = { new Event { IsRead = false } } })
              .Update(new User { Events = { new Event { IsRead = true } } });

            this.SaveUserFileLog(login);
        }

        public void MarkSearchAsRead(string login, string groupName, string robotName, int lastID)
        {
            DB.GetWhere(new User { Login = login,
                                   Groups = { new Group { Name = groupName,
                                                          Robots = { new Robot { Name = robotName,
                                                                                 Author = new Author { IsActive = true } } } } } })
              .Update(new User { Groups = { new Group { Robots = { new Robot { LasDocID = lastID } } } } });

            this.SaveUserFileLog(login);
        }

        public void UpdateRobotPeriod(string login, string groupName, string robotName, string period)
        {
            DB.GetWhere(new User
            {
                Login = login,
                Groups = { new Group { Name = groupName,
                                       Robots = { new Robot { Name = robotName,
                                                              Author = new Author { IsActive = true } } } } }
            })
              .Update(new User { Groups = { new Group { Robots = { new Robot { LasDocID = 0,
                                                                               Period = new Period { Type = period } } } } } });

            this.SaveUserFileLog(login);
        }

        public void AddEvent(Event ev)
        {
            ev.Validate();

            DB.GetWhere(new User { Login = ev.Where.Login })
              .Insert(new User { Events = { ev } },
                      Change.CreateOne(ev, ChangeEnum.Add));

            this.SaveUserFileLog(ev.Where.Login);
        }
    }
}