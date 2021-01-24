using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Text;
using BigDocClient;
using System.Data.SqlClient;
using System.Text.RegularExpressions;
using Booben.Models;

namespace Booben
{
    public class Helper
    {
        public static Dictionary<string, List<Item>> Cache = new Dictionary<string, List<Item>>();

        public static void ClearCache(string login)
        {
            List<string> keysToRemove = new List<string>();

            var e = Cache.GetEnumerator();

            while(e.MoveNext())
            {
                if(e.Current.Key.IndexOf(login + "_") == 0)
                {
                    keysToRemove.Add(e.Current.Key);
                }
            }

            foreach(var key in keysToRemove)
            {
                Cache.Remove(key);
            }
        }

        public static Encoding Encoding = Encoding.GetEncoding("windows-1251");

        public const string FTROBOT_PATH = @"d:\FTRobot\";

        public static string GetDocCode(string code, string dashboardID, string docNumber, int groupNumber)
        {
            if (string.IsNullOrEmpty(dashboardID))
            {
                return code + "-" + int.Parse(docNumber).ToString() + "-" + groupNumber.ToString();
            }
            else
            {
                return code + "_" + dashboardID + "-" + int.Parse(docNumber).ToString() + "-" + groupNumber.ToString();
            }
        }

        public static string GetDirectoryPath(string docNumber)
        {
            return FTROBOT_PATH + (int.Parse(docNumber) - int.Parse(docNumber) % 1000).ToString() + @"\";
        }

        public static string GetFilePath(string code, string dashboardID, string docNumber, int groupNumber)
        {
            return GetDirectoryPath(docNumber) + GetDocCode(code, dashboardID, docNumber, groupNumber) + ".txt";
        }

        public static string ReadWebGroup(string path, Encoding encoding = null)
        {
            if (encoding != null)
            {
                return System.IO.File.ReadAllText(path, encoding);
            }
            else
            {
                return System.IO.File.ReadAllText(path);
            }
        }

        public static void GetUrl(string name,
                                  Item item,
                                  ref string path)
        {
            string[] parts = name.Split(new char[] { '-', '_' });

            if (parts.Length == 3)
            {
                path = GetFilePath(parts[0], null, parts[1], int.Parse(parts[2]));
            }
            else if (parts.Length == 4)
            {
                path = GetFilePath(parts[0], parts[1], parts[2], int.Parse(parts[3]));
            }
            else if (parts.Length == 5)
            {
                path = GetFilePath(parts[0], parts[1] + "-" + parts[2], parts[3], int.Parse(parts[4]));
            }

            if (name.IndexOf("habr") >= 0)
            {
                string id = name.Split('-')[1];
                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\habr\{0}\post_{1}.html", int.Parse(id) / 1000 * 1000 + 1, id);

                item.URL = string.Format("http://habrahabr.ru/post/{0}/", id);
            }
            else if (name.IndexOf("sql") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "windows-1251";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\sql\{0}\tid_{1}_pg_{2}.html", int.Parse(id) / 1000 * 1000 + 1, id, group);

                item.URL = string.Format("http://www.sql.ru/forum/{0}-{1}", id, group);
            }
            else if (name.IndexOf("dou") >= 0)
            {
                string id = name.Split('-')[1];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\dou\{0}\post_{1}.html", int.Parse(id) / 1000 * 1000 + 1, id);

                item.URL = string.Format("http://dou.ua/forums/topic/{0}/", id);
            }
            else if (name.IndexOf("skc") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\skodaclub\{0}\tid_{1}_pg_{2}.html", int.Parse(id) / 1000 * 1000 + 1000, id, group);

                item.URL = string.Format("http://www.skoda-club.org.ua/forum/showthread.php?tid={0}&group={1}", id, group);
            }
            else if (name.IndexOf("search") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\SearchEngines\{0}\tid_{1}_pg_{2}.html", int.Parse(id) / 1000 * 1000 + 1000, id, group);

                item.URL = string.Format("http://searchengines.guru/showthread.php?t={0}&group={1}", id, group);
            }
            else if (name.IndexOf("velo") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\Velokiev\{0}\tid_{1}_pg_{2}.html", int.Parse(id) / 1000 * 1000, id, group);

                item.URL = string.Format("http://www.velokiev.com/forum/viewtopic.php?t={0}&start={1}", id, (int.Parse(group) - 1) * 25);
            }
            else if (name.IndexOf("game") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "windows-1251";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\gamedev\{0}\tid_{1}_pg_{2}.html", int.Parse(id) / 1000 * 1000 + 1000, id, group);

                item.URL = string.Format("http://www.gamedev.ru/forum/?id={0}&group={1}", id, group);
            }
            else if (name.IndexOf("lib") >= 0)
            {
                string id = name.Split('-')[1].Replace("lib", "");

                item.Encoding = "windows-1251";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\librusec\{0}.fb2", id);

                item.URL = string.Empty;
            }
            else if (name.IndexOf("lambda") >= 0)
            {
                string id = name.Split('-')[1];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"i:\lambda\{0}\lambda_{1}.html", int.Parse(id) / 1000 * 1000 + 1, id);

                item.URL = string.Format("http://lambda-the-ultimate.org/node/{0}/", id);
            }
            else if (name.IndexOf("forumua") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\fromua\{0}\tid_{1}_pg_{2}.html", int.Parse(id) / 1000 * 1000 + 1000, int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://forumua.org/viewtopic.php?t={0}&start={1}", id, (int.Parse(group) - 1) * 20);
            }
            else if (name.IndexOf("fxclub") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "windows-1251";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\fxclub\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("00000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://forum.fxclub.org/showthread.php/{0}/group{1}", id, group);
            }
            else if (name.IndexOf("sevinfo2") >= 0 ||
                     name.IndexOf("sevas") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\sevinfo2\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://forum.sevastopol.info/viewtopic.php?t={0}&start={1}", id, (int.Parse(group) - 1) * 25);
            }
            else if (name.IndexOf("sevinfo") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\sevinfo\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://sevpolitforum.ru/viewtopic.php?t={0}&start={1}", id, (int.Parse(group) - 1) * 25);
            }
            else if (name.IndexOf("gotai") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://www.gotai.net/forum/default.aspx?threadid={0}&group={1}", id, group);
            }
            else if (name.IndexOf("pravda") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://forum.pravda.com.ua/index.php?topic={0}.{1}", id, (int.Parse(group) - 1) * 30);
            }
            else if (name.IndexOf("ixbt") >= 0)
            {
                item.Encoding = "windows-1251";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                if (parts.Length == 3)
                {
                    item.URL = string.Format("http://forum.ixbt.com/topic.cgi?id=77:{0}-{1}", parts[1], parts[2]);
                }
                else //if (parts.Length == 4)
                {
                    item.URL = string.Format("http://forum.ixbt.com/topic.cgi?id={0}:{1}-{2}", parts[1], parts[2], parts[3]);
                }
            }
            else if (name.IndexOf("aua") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://forum.autoua.net/showflat.php?Number={0}&fpart={1}", id, group);
            }
            else if (name.IndexOf("fin") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://forum.finance.ua/topic{0}.html?start={1}", id, (int.Parse(group) - 1) * 10);
            }
            else if (name.IndexOf("khaf") >= 0)
            {
                string id = name.Split('-')[1];
                string group = name.Split('-')[2];

                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("https://www.kharkovforum.com/showthread.php?t={0}&group={1}", id, group);
            }
            else if (name.IndexOf("gratis") >= 0)
            {
                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                item.URL = string.Format("http://www.gratis.pp.ru/index.php?s=&act=ST&f={0}&t={1}&st={2}", parts[1], parts[2], parts[3]);
            }
            else if (name.IndexOf("cyber") >= 0)
            {
                item.Encoding = "UTF-8";
                item.FileEncoding = "windows-1251";

                //path = string.Format(@"d:\gotai\{0}\tid_{1}_pg_{2}.html", (int.Parse(id) / 1000 * 1000 + 1000).ToString("000000"), int.Parse(id).ToString("0000000"), int.Parse(group).ToString("0000"));

                if (parts.Length == 4)
                {
                    item.URL = string.Format("http://www.cyberforum.ru/{0}/thread{1}-group{2}.html", parts[1], parts[2], parts[3]);
                }
                else
                {
                    item.URL = string.Format("http://www.cyberforum.ru/{0}-{1}/thread{2}-group{3}.html", parts[1], parts[2], parts[3], parts[4]);
                }
            }
            else
            {
                throw new Exception("Is not valid name " + name);
            }
        }
        
        public static bool IsSmile(string href)
        {
            if (href.IndexOf("searchengines.guru") >= 0 ||
                href.IndexOf("dou.ua") >= 0 ||
                href.IndexOf("gamedev.ru") >= 0 ||
                href.IndexOf("habrahabr.ru") >= 0 ||
                href.IndexOf("gotai.net") >= 0 ||
                href.IndexOf("sql.ru") >= 0 ||
                href.IndexOf("sevastopol.info") >= 0 ||
                href.IndexOf("skoda-club.org.ua") >= 0 ||
                href.IndexOf("pravda.com.ua") >= 0 ||
                href.IndexOf("forum.ixbt.com") >= 0 ||
                href.IndexOf("forumimg.net") >= 0 ||
                href.IndexOf("autoua.net") >= 0 ||
                href.IndexOf("kharkovforum.com") >= 0 ||
                href.IndexOf("gratis.pp.ru") >= 0 ||
                href.IndexOf("finance.ua") >= 0 ||
                href.IndexOf("cyberforum.ru") >= 0 ||
                href.IndexOf("smil") >= 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public static string GetTitle(string html)
        {
            Match m = System.Text.RegularExpressions.Regex.Match(html, "<title[^>]*>(?<title>(.*?))</title>", RegexOptions.IgnoreCase);

            if (m.Success)
            {
                return m.Groups["title"].Value;
            }
            else
            {
                return string.Empty;
            }
        }

        private static string RemoveTagBlock(string html, string startBlock, string endBlock)
        {
            //remove scripts
            while (true)
            {
                int idx1 = html.IndexOf(startBlock);
                int idx2 = html.IndexOf(endBlock);

                if (idx1 >= 0 && idx2 >= 0)
                {
                    html = html.Remove(idx1, idx2 - idx1 + endBlock.Length);
                }
                else
                {
                    break;
                }
            }

            return html;
        }

        public static string GetTitleWords(string title)
        {
            if (!string.IsNullOrEmpty(title))
            {
                string titleWords = string.Empty;

                MatchCollection mc = Regex.Matches(title, "(?<word>[\\@a-zA-Zа-яА-Я0-9]{3,})");

                foreach (Match m in mc)
                {
                    if (m.Success)
                    {
                        string value = m.Groups["word"].Value.ToLower();
                        if (value[0] != '@' &&
                           value != "ssearch" &&
                           value != "svelo" &&
                            value != "spravda" &&
                            value != "ssql" &&
                            value != "sgame" &&
                            value != "sixbt" &&
                            value != "sskc" &&
                            value != "shabr" &&
                            value != "sdou" &&
                            value != "sgotai" &&
                            value != "ssevas" &&
                            value != "saua" &&
                            value != "sfin" &&
                            value != "skhaf")
                        {
                            titleWords += "_" + value + " ";
                        }
                        else
                        {
                            titleWords += value + " ";
                        }
                    }
                }

                return titleWords;
            }
            else
            {
                return title;
            }
        }

        public static string StripTags(string html)
        {
            StringBuilder sb = new StringBuilder();

            html = RemoveTagBlock(html, "<title", "</title>");
            html = RemoveTagBlock(html, "<script", "</script>");

            bool isTag = html.IndexOf("<") > html.IndexOf(">");

            foreach (char c in html)
            {
                if (c == '<')
                {
                    isTag = true;
                    continue;
                }
                else if (c == '>')
                {
                    isTag = false;
                }
                else if (!isTag)
                {
                    sb.Append(c);
                }
            }

            html = sb.ToString();

            html = Regex.Replace(html, @"\n|\t|\r", " ");
            html = Regex.Replace(html, @"\s+", " ");

            return html;
        }

        private static void SetWeight(string text,
                                      string word,
                                      int[] dic,
                                      int weight,
                                      bool isPhrase,
                                      ref int maxWeight,
                                      ref int maxWeightIdx)
        {
            Regex regex;

            //if (word.Length < 8)
            //{
            //    regex = new Regex("[^a-zA-Zа-яА-Я0-9]" + word + "[^a-zA-Zа-яА-Я0-9]");
            //}
            //else
            //{
            regex = new Regex("[^a-zA-Zа-яА-Я0-9]" + word);
            //}

            MatchCollection mc = regex.Matches(text);

            if (!isPhrase)
            {
                foreach (Match m in mc)
                {
                    int idx = m.Index >> 8;

                    dic[idx] += weight;

                    if (dic[idx] >= maxWeight)
                    {
                        maxWeight = dic[idx];
                        maxWeightIdx = idx;
                    }
                }
            }
            else
            {
                List<int> usedIdx = new List<int>();

                foreach (Match m in mc)
                {
                    int idx = m.Index >> 8;

                    if (usedIdx.IndexOf(idx) == -1)
                    {
                        dic[idx] += weight;

                        if (dic[idx] >= maxWeight)
                        {
                            maxWeight = dic[idx];
                            maxWeightIdx = idx;
                        }

                        usedIdx.Add(idx);
                    }
                }
            }
        }

        public static string GetTextSnippet(string matchWords, string text, bool isPhrase)
        {
            int[] dic = new int[(text.Length >> 8) + 1]; //max 256 bytes

            text = text.ToLower();

            int maxWeight = 0;
            int maxWeightIdx = 0;

            string[] words = matchWords.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            for (int i = 0; i < words.Length; i++)
            {
                SetWeight(text, words[i], dic, words.Length - i, isPhrase, ref maxWeight, ref maxWeightIdx);
            }

            maxWeightIdx <<= 8;

            if (text.Length > maxWeightIdx + 256)
            {
                text = " " + text.Substring(maxWeightIdx, 256) + " ";
            }
            else
            {
                text = " " + text.Substring(maxWeightIdx, text.Length - maxWeightIdx) + " ";
            }

            foreach (string word in words.OrderByDescending(x => x.Length))
            {
                //if (word.Length < 8)
                //{
                //    text = Regex.Replace(text, @"[^a-zA-Zа-яА-Я0-9<>]" + word + @"[^a-zA-Zа-яА-Я0-9<>]", " <b>" + word + "</b> ", RegexOptions.IgnoreCase);
                //}
                //else
                //{
                text = Regex.Replace(text, @"[^a-zA-Zа-яА-Я0-9<>]" + word, " <b>" + word + "</b>", RegexOptions.IgnoreCase);
                //}
            }

            return text;
        }

        public static string GetHtmlSnippet(string matchWords,
                                            Item item,
                                            string html,
                                            bool isPhrase,
                                            bool onlySubject = false)
        {
            string text = string.Empty;

            try
            {
                text = GetTextSnippet(matchWords, StripTags(html), isPhrase);
            }
            catch (Exception ex)
            {
                text = "Ошибка загрузки контента";
            }

            return text;
        }

        public static void InsertQuery(string ip,
                                       string group,
                                       string site,
                                       string query1,
                                       string query2,
                                       string action)
        {
            using (SqlConnection con = new SqlConnection("Server=localhost;Database=Booben;User Id=sa;Password=1;"))
            {
                //
                // Open the SqlConnection.
                //
                con.Open();
                //
                // The following code uses an SqlCommand based on the SqlConnection.
                //
                using (SqlCommand command = new SqlCommand("INSERT INTO Query (IP, Group, Site, Query1, Query2, [Action]) VALUES (@IP, @Group, @Site, @Query1, @Query2, @Action)", con))
                {
                    command.Parameters.AddWithValue("@IP", ip);
                    command.Parameters.AddWithValue("@Group", group ?? Convert.DBNull);
                    command.Parameters.AddWithValue("@Site", site ?? Convert.DBNull);
                    command.Parameters.AddWithValue("@Query1", query1 ?? Convert.DBNull);
                    command.Parameters.AddWithValue("@Query2", query2 ?? Convert.DBNull);
                    command.Parameters.AddWithValue("@Action", action ?? Convert.DBNull);

                    command.ExecuteNonQuery();

                    //using (SqlDataReader reader = command.ExecuteReader())
                    //{
                    //    while (reader.Read())
                    //    {
                    //        Console.WriteLine("{0} {1} {2}",
                    //        reader.GetInt32(0), reader.GetString(1), reader.GetString(2));
                    //    }
                    //}
                }
            }
        }
    }
}