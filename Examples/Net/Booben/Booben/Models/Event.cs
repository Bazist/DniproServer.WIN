using Booben.Controllers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Event
    {
        public Event()
        {
        }

        public bool? IsRead { get; set; }
        public DateTime? OnDate { get; set; }

        public WhereEvent Where { get; set; }

        //Added comment, Like, Changed robot
        public WhatEvent What { get; set; }

        public object GetWhereObject()
        {
            object item = null;

            DAL dal = new DAL();

            if (Where.Type == "User")
            {
                item = dal.GetUserByLogin(Where.Login, 3);
            }
            else if (Where.Type == "Group")
            {
                item = dal.GetGroup(Where.Login, Where.GroupName, 3);
            }
            else if (Where.Type == "Robot")
            {
                item = dal.GetRobot(Where.Login, Where.GroupName, Where.RobotName, 3);
            }
            else if (Where.Type == "Library")
            {
                item = dal.GetLibrary(Where.Login, Where.LibName, 3);
            }
            else if (Where.Type == "Comment")
            {
                if (!string.IsNullOrEmpty(Where.LibName))
                {
                    item = dal.GetLibraryComment(Where.Login, Where.LibName, Where.CommentDate);
                }
                else if (!string.IsNullOrEmpty(Where.RobotName))
                {
                    item = dal.GetRobotComment(Where.Login, Where.GroupName, Where.RobotName, Where.CommentDate);
                }
            }
            
            return item;
        }

        public static Event Create(string login,
                                   string whereType,
                                   string whereLogin,
                                   string whereGroupName,
                                   string whereRobotName,
                                   string whereLibName,
                                   DateTime whereCommentDate,
                                   string whatType,
                                   string whatLogin,
                                   string whatMessage)
        {
            Event ev = new Event()
            {
                IsRead = false,
                OnDate = DateTime.Now,

                Where = new WhereEvent()
                {
                    Type = whereType,
                    Login = whereLogin,
                    GroupName = whereGroupName,
                    RobotName = whereRobotName,
                    LibName = whereLibName,
                    CommentDate = whereCommentDate
                },

                What = new WhatEvent()
                {
                    Type = whatType,
                    Login = whatLogin,
                    Message = whatMessage
                }
            };

            return ev;
        }

        public void Validate()
        {
            if (Where != null)
            {
                Where.Validate();
            }

            if (What != null)
            {
                What.Validate();
            }
        }
    }
}