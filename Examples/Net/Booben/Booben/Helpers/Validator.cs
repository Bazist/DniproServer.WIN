using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Text;
using BigDocClient;
using System.Text.RegularExpressions;

namespace Booben
{
    public class Validator
    {
        public static void ValidateName(string name,
                                        string errMessage)
        {
            if (!string.IsNullOrEmpty(name))
            {
                Regex regex = new Regex("^[a-zA-Z0-9\\s]{3,16}$");

                if (!regex.IsMatch(name))
                {
                    throw new Exception(errMessage);
                }
            }
        }

        public static void ValidateText(string text,
                                        string errMessage)
        {
            if (!string.IsNullOrEmpty(text))
            {
                if (text.Contains("<") || text.Contains(">") || text.Contains("'") || text.Contains('"'))
                {
                    throw new Exception(errMessage);
                }
            }
        }

        public static void ValidateNumber(int? number,
                                          string errMessage,
                                          int minValue = 0,
                                          int maxValue = int.MaxValue)
        {
            if (number != null)
            {
                if (number < minValue || number > maxValue)
                {
                    throw new Exception(errMessage);
                }
            }
        }

        public static void ValidateEmail(string email,
                                         string errMessage)
        {
            if (!string.IsNullOrEmpty(email))
            {
                Regex regex = new Regex("^[a-zA-Z0-9\\.]{3,32}\\@[a-zA-Z0-9\\.]{3,32}$");

                if (!regex.IsMatch(email))
                {
                    throw new Exception(errMessage);
                }
            }
        }

        public static void Required(object obj, string errMessage)
        {
            if(obj == null || string.IsNullOrEmpty(obj.ToString()))
            {
                throw new Exception(errMessage);
            }
        }
    }
}