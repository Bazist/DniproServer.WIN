using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Reference
    {
        public bool? IsActive { get; set; }
        public string Login { get; set; }
        public string GroupName { get; set; }
        public string RobotName { get; set; }
        public string LibName { get; set; }

        public void Validate()
        {
            Validator.ValidateName(Login, "Login is not valid");
            Validator.ValidateName(GroupName, "Group Name is not valid");
            Validator.ValidateName(RobotName, "Robot Name is not valid");
            Validator.ValidateName(LibName, "Lib Name is not valid");
        }
    }
}