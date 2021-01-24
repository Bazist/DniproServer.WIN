using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Booben.Models
{
    public class Group
    {
        public Group()
        {
            Robots = new List<Robot>();
            Notifications = new List<Notification>();
        }

        public Author Author { get; set; }

        public Reference Reference { get; set; }

        public string Name { get; set; }
        public string Description { get; set; }

        public List<Robot> Robots { get; set; }

        public List<Notification> Notifications { get; set; }

        public string GetDescription()
        {
            return "Group descr";
        }

        public string GetNameOfNewRobot()
        {
            string baseTemplate = "New Robot";

            int idx = 0;

            while (true)
            {
                string nameOfNewRobot;

                if (idx == 0)
                {
                    nameOfNewRobot = baseTemplate;
                }
                else
                {
                    nameOfNewRobot = baseTemplate + " " + idx.ToString();
                }

                if (!Robots.Exists(x => x.Name == nameOfNewRobot))
                {
                    return nameOfNewRobot;
                }

                idx++;
            }
        }

        public void Validate()
        {
            if (Author != null)
            {
                Author.Validate();
            }

            if (Reference != null)
            {
                Reference.Validate();
            }

            Validator.ValidateName(Name, "Name is not valid");
            Validator.ValidateText(Description, "Description is not valid");

            foreach(Robot wid in Robots)
            {
                wid.Validate();
            }

            foreach(Notification not in Notifications)
            {
                not.Validate();
            }
        }
    }
}