using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Web;

namespace Booben.Models
{
    public class Robot
    {
        public Robot()
        {
            Conditions = new List<Condition>();
            Comments = new List<Comment>();
            Subscribers = new List<Subscriber>();
        }

        public Author Author { get; set; }

        public string Name { get; set; }
        public string Description { get; set; }
        public byte[] Picture { get; set; }
        
        public Period Period { get; set; }
        public int? LasDocID { get; set; }
        
        public Source Source { get; set; }
        public List<Condition> Conditions { get; set; }
        public Output Output { get; set; }
        
        public Price Price { get; set; }

        public void Validate()
        {
            if (Author != null)
            {
                Author.Validate();
            }

            Validator.ValidateName(Name, "Name is not valid");
            Validator.ValidateText(Description, "Description is not valid");

            if (Period != null)
            {
                Period.Validate();
            }

            if (Source != null)
            {
                Source.Validate();
            }

            foreach(Condition con in Conditions)
            {
                con.Validate();
            }

            if (Output != null)
            {
                Output.Validate();
            }

            if (Price != null)
            {
                Price.Validate();
            }

            foreach (Comment com in Comments)
            {
                com.Validate();
            }
        }

        public List<Comment> Comments { get; set; }
        public List<Subscriber> Subscribers { get; set; }

        public List<Item> SearchItems(string login = null,
                                      string groupName = null,
                                      bool useCache = true)
        {
            string key = string.Format("{0}_{1}_{2}", login, groupName, Name);

            if(useCache && Helper.Cache.ContainsKey(key))
            {
                return Helper.Cache[key];
            }

            List<Item> items = new List<Item>();

            FTSearchService7.FTSearchServiceClient client = new FTSearchService7.FTSearchServiceClient();
            var selectors = CreateSelectors(Conditions[0]);

            bool hasTextTag = (this.Output.Type == "text");
            bool hasCoubTag = (this.Output.Type == "coub");
            bool hasImageTag = (this.Output.Type == "image");
            bool hasGifTag = (this.Output.Type == "gif");
            bool hasVideoTag = (this.Output.Type == "video");

            string matchWords = string.Empty;

            foreach (Condition condition in Conditions)
            {
                if (!string.IsNullOrEmpty(condition.Phrase))
                {
                    matchWords += condition.Phrase + " ";
                }

                if (!string.IsNullOrEmpty(condition.Words))
                {
                    matchWords += condition.Words + " ";
                }
            }

            string[] queryResult = client.SearchQuery(selectors.ToArray(), Convert.ToUInt32(LasDocID ?? 0), int.MaxValue, 0, hasTextTag);
            
            //fill data table
            foreach (string queryRow in queryResult)
            {
                Item item = new Item();
                item.Author = new Author { IsActive = true };

                string[] values = queryRow.Split(';');

                string result = string.Empty;
                string path = string.Empty;

                Helper.GetUrl(values[0], item, ref path);

                //first column
                //row[0] = string.Format("<a href={0}>{1}</a>", rvi.Url, values[0]);

                List<string> hrefs = new List<string>(); //ClientHrefs[clientIPAddress];

                string persMatchWords = string.Empty;

                for (int i = 1; i < values.Length; i++)
                {
                    if (!string.IsNullOrEmpty(values[i]))
                    {
                        persMatchWords += values[i] + " ";
                    }
                }

                string html = Helper.ReadWebGroup(path, Encoding.GetEncoding(item.FileEncoding));

                if (html.IndexOf("</title>") > 0)
                {
                    html = html.Substring(0, html.IndexOf("</title>") + 8); //skip tags
                                                                            //html = html.Replace("ssql", "").Replace("sdou", "").Replace("sgame", "").Replace("ssearch", "").Replace("shabr", "");
                }

                item.Title = Helper.GetTitle(html);

                MatchCollection mc = null;

                if (hasImageTag || hasGifTag || hasVideoTag || hasCoubTag)
                {
                    Regex reg = null;
                    
                    if (hasImageTag)
                    {
                        reg = new Regex("<img[^!]+src=[\"'](?<href>http[s]?:[^\"']+\\.jpg)[\"']");
                    }
                    else if (hasGifTag)
                    {
                        reg = new Regex("<img[^!]+src=[\"'](?<href>http[s]?:[^\"']+\\.gif)[\"']");
                    }
                    else if (hasVideoTag)
                    {
                        reg = new Regex("link=https://www.youtube.com/watch\\?v=(?<href>[^\"']+)\"");

                        mc = reg.Matches(html);

                        if (mc.Count == 0)
                        {
                            reg = new Regex("youtube.com/embed/(?<href>[^\"']+)\"");
                        }
                    }
                    else if (hasCoubTag)
                    {
                        reg = new Regex("src=\"http://coub.com/embed/(?<href>[^\"']+)\"");

                        mc = reg.Matches(html);

                        if (mc.Count == 0)
                        {
                            reg = new Regex("http://coub.com/view/(?<href>[^\"']+)\"");
                        }
                    }

                    mc = reg.Matches(html);

                    string snippet = string.Empty;

                    if (mc.Count > 0)
                    {
                        foreach (Match match in mc)
                        {
                            string href = match.Groups["href"].Value;

                            if (hrefs.IndexOf(href) == -1)
                            {
                                if (!string.IsNullOrEmpty(href))
                                {
                                    if (hasImageTag)
                                    {
                                        snippet += string.Format("<br/><img width='500px' src='{0}'/>", href);
                                    }
                                    else if (hasGifTag)
                                    {
                                        if (!Helper.IsSmile(href))
                                        {
                                            snippet += string.Format("<br/><img onload='checkSize(this);' width='500px' src='{0}'/>", href);
                                        }
                                        else
                                        {
                                            continue;
                                        }
                                    }
                                    else if (hasVideoTag)
                                    {
                                        snippet += string.Format("<br/><iframe width='640' height='360' src='http://www.youtube.com/embed/{0}' frameborder='0' allowfullscreen></iframe>", href);
                                    }
                                    else if (hasCoubTag)
                                    {
                                        snippet += string.Format("<br/><iframe width='640' height='360' src='http://coub.com/embed/{0}' frameborder='0' allowfullscreen></iframe>", href);
                                    }
                                }

                                hrefs.Add(href);
                            }
                        }

                        if (!string.IsNullOrEmpty(snippet))
                        {
                            item.Text = snippet;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue; //skip error
                    }
                }
                else
                {
                    item.Text = Helper.GetHtmlSnippet(matchWords + persMatchWords, item, html, false);
                }

                item.Index = items.Count;

                items.Add(item);
            }

            if (useCache)
            {
                Helper.Cache.Add(key, items);
            }

            return items;
        }

        private List<FTSearchService7.FTSearchSelector> CreateSelectors(Condition condition)
        {
            string tags = string.Empty;

            if (condition.OnlyTitle ?? false)
            {
                condition.Phrase = Helper.GetTitleWords(condition.Phrase);
                condition.Words = Helper.GetTitleWords(condition.Words);
            }

            //period type
            if(Period.Type == "day")
            {
                condition.Phrase += string.Format(" y{0} m{1} d{2}",
                                                    DateTime.Now.Year.ToString("0000"),
                                                    DateTime.Now.Month.ToString("00"),
                                                    DateTime.Now.Day.ToString("00"));
            }
            else if(Period.Type == "week")
            {
                condition.Phrase += string.Format(" y{0} w{1}",
                                                    DateTime.Now.Year.ToString("0000"),
                                                    (DateTime.Now.DayOfYear / 7 + 1).ToString("00"));
            }
            else if (Period.Type == "month")
            {
                condition.Phrase += string.Format(" y{0} m{1}",
                                                DateTime.Now.Year.ToString("0000"),
                                                DateTime.Now.Month.ToString("00"));

            }
            else if (Period.Type == "year")
            {
                condition.Phrase += string.Format(" y{0}",
                                                DateTime.Now.Year.ToString("0000"));
            }

            //output type
            if (Output.Type == "image")
            {
                condition.Phrase += " img src http jpg";
            }
            else if (Output.Type == "gif")
            {
                condition.Phrase += " bgif";
            }
            else if (Output.Type == "video")
            {
                condition.Phrase += " youtube embed";
            }
            else if (Output.Type == "coub")
            {
                condition.Phrase += " coub http";
            }

            List<FTSearchService7.FTSearchSelector> selectors = new List<FTSearchService7.FTSearchSelector>();

            if (!string.IsNullOrEmpty(condition.Phrase))
            {
                FTSearchService7.FTSearchSelector selector = new FTSearchService7.FTSearchSelector();

                selector.SelectorType = 1;

                selector.Name = "CustomPhrase";
                selector.AgregationType = 1;

                selector.ConditionType = 1;
                selector.Operand2 = 0;

                selector.FilePath = "";

                selector.RangeType = 0;
                selector.MinBound = condition.Phrase;
                selector.MaxBound = "";

                selector.AgregationSortType = 4;
                selector.IsSortAsc = false;

                selectors.Add(selector);
            }

            if (!string.IsNullOrEmpty(condition.Words))
            {
                FTSearchService7.FTSearchSelector selector = new FTSearchService7.FTSearchSelector();

                selector.Name = "CustomList";
                selector.AgregationType = 1;
                selector.AgregationSortType = 4;
                selector.IsSortAsc = false;

                selector.ConditionType = 6;
                selector.Operand2 = Convert.ToUInt32(condition.MinWords ?? 0);

                selector.FilePath = "";

                selector.RangeType = 0;
                selector.MinBound = condition.Words;
                selector.MaxBound = "";

                selector.SelectorType = 1;

                selectors.Add(selector);
            }

            if (!string.IsNullOrEmpty(condition.Library.LibName))
            {
                FTSearchService7.FTSearchSelector selector = new FTSearchService7.FTSearchSelector();

                selector.Name = condition.Library.LibName;
                selector.AgregationType = 1;

                selector.ConditionType = 6;
                selector.Operand2 = 0;

                selector.AgregationSortType = 1;
                selector.IsSortAsc = false;

                //selector.FilePath = SELECTORS_PATH + rr.CategoryNameManyItemsFromCategory + ".txt";

                //temporary
                //this.StemSelectorFile(selector.FilePath);

                selector.RangeType = 0;
                selector.MinBound = "";
                selector.MaxBound = "";

                selector.SelectorType = 1;

                selectors.Add(selector);
            }

            return selectors;
        }

        public int GetAmountNewItems()
        {
            return 0;
        }
    }
}