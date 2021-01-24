using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Controllers
{
    public class Context
    {
        public string Operation;
        public string SecKey;
        public string Login;
        public string MyLogin;
        public string GroupName;
        public string RobotName;
        public string LibName;

        private Context()
        {

        }

        public static Context GetContext(HttpRequestBase request)
        {
            Context context = new Context();

            context.Operation = request["context_operation"];
            context.SecKey = request["context_seckey"];
            context.Login = request["context_login"];
            context.MyLogin = request["context_mylogin"];
            context.GroupName = request["context_groupname"];
            context.RobotName = request["context_Robotname"];
            context.LibName = request["context_libname"];

            return context;
        }

        public void SaveViewBag(dynamic viewBag)
        {
            viewBag.Operation = this.Operation;
            viewBag.SecKey = this.SecKey;
            viewBag.Login = this.Login;
            viewBag.MyLogin = this.MyLogin;
            viewBag.GroupName = this.GroupName;
            viewBag.RobotName = this.RobotName;
            viewBag.LibName = this.LibName;
        }
    }
}