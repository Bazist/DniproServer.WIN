using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class WhereEvent
    {
        public string Type { get; set; }
        public string Login { get; set; }
        public string GroupName { get; set; }
        public string RobotName { get; set; }
        public string LibName { get; set; }
        public DateTime? CommentDate { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Type, "Type is not valid");
            Validator.ValidateName(Login, "Login is not valid");
            Validator.ValidateName(GroupName, "Message is not valid");
            Validator.ValidateName(RobotName, "Robot name is not valid");
            Validator.ValidateName(LibName, "Library name is not valid");
        }
    }
}