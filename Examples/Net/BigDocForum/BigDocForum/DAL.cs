using BigDocClient;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace BigDocForum
{
    public class DAL
    {
        public static BigDoc _db = new BigDoc("127.0.0.1", 4477);

        public static int GetCountMessage(int articleId)
        {
            return int.Parse(_db.GetWhere(new { ArticleId = articleId }).Value("{'CountMessage':$}"));
        }
    }
}