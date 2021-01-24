/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: BigDoc@gmail.com, skype: vyacheslavm81)
# This file is part of BigDocClient.
#
# BigDocClient is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# BigDocClient is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Runtime.Serialization;

namespace BigDocClient
{
    public class Tests
    {
        [DataContract]
        class TO
        {
            [DataMember(IsRequired = true)]
            public int id { get; set; }

            [DataMember(IsRequired = true)]
            public string FirstName { get; set; }

            [DataMember(IsRequired = true)]
            public string LastName { get; set; }

            [DataMember(IsRequired = true)]
            public List<int> Arr { get; set; }
        }

        public static void Clear(BigDoc db)
        {
            db.Clear();
        }

        public static void Test0(BigDoc db)
        {
            List<string> docs = new List<string>();
            for (int i = 0; i < 100000; i++)
            {
                docs.Add("{'id':" + i.ToString() + ",'FirstName':'John" + i.ToString() + "','LastName':'Smith" + i.ToString() + "','arr':[1,2,3,4,5]}");
            }

            Stopwatch s = new Stopwatch();
            s.Start();

            db.AddDocs(docs);

            s.Stop();

            Console.WriteLine("Test0: " + s.ElapsedMilliseconds.ToString());
        }

        public static void Test1(BigDoc db)
        {
            Stopwatch s = new Stopwatch();

            s.Start();

            ulong count = 0;

            for (int i = 0; i < 1000; i++)
            {
                count += db.GetWhere("{'Name':'Bill'}").Count();
            }

            s.Stop();

            Console.WriteLine("Test1: " + s.ElapsedMilliseconds.ToString() + "-" + count.ToString());
        }

        public static void Test2(BigDoc db)
        {
            db.AddDoc("{'login':'michael'}");
            db.AddDoc("{'login':'micha'}");
            db.AddDoc("{'login':'mira'}");
            db.AddDoc("{'login':'frank'}");

            db.GetWhere("{'login':@'mi*'}").Print("{'login':$}");
        }

        public static void Test3(BigDoc db)
        {
            db.AddDoc("{'arr':['1','2','3']}");
            db.AddDoc("{'arr':['3','4','2']}");
            db.AddDoc("{'arr':['2','3','4']}");
            db.AddDoc("{'arr':['3','4','5']}");

            db.GetWhere("{'arr':[In,'2','3']}").Print("{'arr':[$, $, $]}");
        }

        public static void Test4(BigDoc db)
        {
            db.AddDoc("{'arr':'1'}");
            db.AddDoc("{'arr':'3'}");
            db.AddDoc("{'arr':'2'}");
            db.AddDoc("{'arr':'3'}");

            //string json = db.GetPartDoc("{'Array':[$,$]}", docID);

            db.GetWhere("{'arr':@'1-2'}").Delete("{'arr':$]}");
        }

        public static void Test5(BigDoc db)
        {
            db.AddDoc("{'firstName':'John','lastName':'Smith', 'Value':'1'}");
            db.AddDoc("{'firstName':'John','lastName':'Dereck', 'Value':'2'}");
            db.AddDoc("{'firstName':'John','lastName':'Connor', 'Value':'3'}");
            db.AddDoc("{'firstName':'John','lastName':'Smith', 'Value':'4'}");

            db.GetWhere("{'firstName':'John','lastName':'Smith'}").Print("{'Value':$]}");
        }

        public static void Test6(BigDoc db)
        {
            TO[] docs = new TO[50000];

            for (int i = 0; i < 50000; i++)
            {
                docs[i] = new TO
                {
                    id = i,
                    FirstName = "John" + i.ToString(),
                    LastName = "Smith" + i.ToString(),
                    Arr = new List<int>() { 1, 2, 3, 4, 5 }
                };
            }

            Stopwatch s = new Stopwatch();
            s.Start();

            db.AddDocs<TO>(docs);

            s.Stop();

            Console.WriteLine("Test6: " + s.ElapsedMilliseconds.ToString());
        }

        public static void Test7(BigDoc db)
        {
            TO to1 = new TO
            {
                id = 123,
                FirstName = "John",
                LastName = "Smith",
                Arr = new List<int>() { 1, 2, 3, 4, 5 }
            };

            db.AddDoc<TO>(to1);

            TO to2 = new TO
            {
                id = 123,
                FirstName = "John",
                LastName = "Smith",
                Arr = new List<int>() { 7, 4, 2, 8, 9 }
            };

            db.AddDoc<TO>(to2);

            TO[] tos = db.GetWhere("{'Arr':[In,2,4]}").Select<TO>();

            //tos = tos;
        }

        public enum Blaha
        {
            bla1,
            bla2
        }

        [DataContract]
        class PlanOption
        {
            [DataMember(IsRequired = true)]
            public Blaha Prop { get; set; }

            [DataMember(IsRequired = true)]
            public int ServicePlanID { get; set; }

            [DataMember(IsRequired = true)]
            public int ServicePlanOption { get; set; }

            [DataMember(IsRequired = true)]
            public bool IsDefault { get; set; }
        }

        public static void Test8(BigDoc db)
        {
            PlanOption[] objs = new PlanOption[500000];

            for (int i = 0; i < 500000; i++)
            {
                objs[i] = new PlanOption() { Prop = Blaha.bla1, ServicePlanID = i, ServicePlanOption = i, IsDefault = true };
            }

            Stopwatch s = new Stopwatch();
            s.Start();

            db.AddDoc(new { hello = "ok" });

            db.AddDocs<PlanOption>(objs);

            s.Stop();

            Console.WriteLine("Inserts: " + s.ElapsedMilliseconds.ToString());

            s = new Stopwatch();
            s.Start();

            objs = db.GetWhere("{'IsDefault':true}").Select<PlanOption>();

            s.Stop();

            Console.WriteLine("Read all: " + s.ElapsedMilliseconds.ToString());
        }

        [DataContract]
        public class Page
        {
            [DataMember(IsRequired = true)]
            public int PageId { get; set; }

            [DataMember(IsRequired = true)]
            public List<Comment> Childs { get; set; }
        }

        [DataContract]
        public class Comment
        {
            [DataMember(IsRequired = true)]
            public string Text { get; set; }

            [DataMember(IsRequired = true)]
            public List<Comment> Childs { get; set; }
        }

        public static void Test9(BigDoc db)
        {
            List<Comment> level1 = new List<Comment>();
            List<Comment> level2 = new List<Comment>();
            List<Comment> level3 = new List<Comment>();

            //initialize data, 10*10*10 comments in each page
            for (int i = 0; i < 10; i++)
            {
                level3.Add(
                    new Comment()
                    {
                        Text = "Blah" + i.ToString()
                    });
            }

            for (int i = 0; i < 10; i++)
            {
                level2.Add(
                    new Comment()
                    {
                        Text = "Blah" + i.ToString(),
                        Childs = level3
                    });
            }

            for (int i = 0; i < 10; i++)
            {
                level1.Add(
                    new Comment()
                    {
                        Text = "Blah" + i.ToString(),
                        Childs = level2
                    });
            }

            Page[] pages = new Page[2500];

            for (int i = 0; i < 2500; i++)
            {
                pages[i] = new Page();
                pages[i].PageId = i;
                pages[i].Childs = level1;
            }

            //insert pages with comments;
            Stopwatch s = new Stopwatch();

            s.Start();

            for (int i = 0; i < 2500; i++)
            {
                //db.AddDoc<Page>(pages[i]);
            }

            s.Stop();

            Console.WriteLine("Inserts: " + s.ElapsedMilliseconds.ToString());

            //read pages with comments
            s = new Stopwatch();
            s.Start();

            for (int i = 0; i < 1000; i++)
            {
                Page[] page = db.GetWhere("{'PageId':1}").Select<Page>();

                //PartDoc[] page = db.GetWhere("{'PageId':1}").Select("{'Childs':[R,{'Text':$,'Childs':[R,{'Text':$,'Childs':[R,{'Text':$}]}]}]}");

                //do something with page
                //...
            }

            s.Stop();

            Console.WriteLine("Read all: " + s.ElapsedMilliseconds.ToString());
        }

        public class SerTest
        {
            [DataMember(IsRequired = true)]
            public List<int> Arr { get; set; }
        }

        public static void Test10(BigDoc db)
        {
            SerTest st = new SerTest();
            st.Arr = new List<int>();

            for (int i = 0; i < 100; i++)
            {
                st.Arr.Add(i);
            }

            string str = "";// = Serialization.Serialize(st);

            int code = 0;

            //insert pages with comments;
            Stopwatch s = new Stopwatch();

            s.Start();

            for (int i = 0; i < 100000; i++)
            {
                code += Serialization.DeserializeProxy<SerTest>(new SerTest(), str).GetHashCode();
                //code += Serialization.Deserialize<SerTest>(str).GetHashCode();
            }

            s.Stop();

            Console.WriteLine("Inserts: " + s.ElapsedMilliseconds.ToString());
        }

        [DataContract]
        private class Person
        {
            [DataMember(IsRequired = true)]
            public int UserId { get; set; }

            [DataMember(IsRequired = true)]
            public string FirstName { get; set; }

            [DataMember(IsRequired = true)]
            public string LastName { get; set; }

            [DataMember(IsRequired = true)]
            public List<int> Arr { get; set; }
        }

        public static void Test11(BigDoc db)
        {
            Person[] docs = new Person[100000];

            for (int i = 0; i < 100000; i++)
            {
                docs[i] = new Person
                {
                    UserId = i,
                    FirstName = "Заец",
                    LastName = "Зайцев" + i.ToString() + "-й",
                    Arr = new List<int>() { 1, 2, 3, 4, 5 }
                };
            }

            Stopwatch s = new Stopwatch();
            s.Start();

            db.AddDocs<Person>(docs);

            s.Stop();

            Console.WriteLine("Test11: " + s.ElapsedMilliseconds.ToString());
        }

        public static void Test12(BigDoc db)
        {
            PartDoc[] docs = new PartDoc[100000];

            for (int i = 0; i < 100000; i++)
            {
                docs[i].Json = "{'UserId':" + i.ToString() + ",'FirstName':'Заец','LastName':'Зайцев" + i.ToString() + "-й','Arr':[1,2,3,4,5]}";
            }

            Stopwatch s = new Stopwatch();
            s.Start();

            db.AddDocs(docs);

            s.Stop();

            Console.WriteLine("Test12: " + s.ElapsedMilliseconds.ToString());
        }

        public static void Test13(BigDoc db)
        {
            PartDoc[] docs = new PartDoc[100000];

            for (int i = 0; i < 100000; i++)
            {
                docs[i].Json = "{'UserId':" + i.ToString() + ",'FirstName':'Заец','LastName':'Зайцев" + i.ToString() + "-й','Arr':[1,2,3,4,5]}";
            }

            Stopwatch s = new Stopwatch();
            s.Start();

            for (int i = 0; i < 100000; i++)
            {
                db.AddDoc(docs[i]);
            }

            s.Stop();

            Console.WriteLine("Test13: " + s.ElapsedMilliseconds.ToString());
        }

        public class Address
        {
            public string Country { get; set; }
            public string City { get; set; }
        }

        public class User
        {
            public string FirstName { get; set; }
            public string LastName { get; set; }
            public Address Address { get; set; }
        }

        public static void Test14(BigDoc db)
        {
            Address address = new Address() { Country = "United Kingdom", City = "London" };

            User user = new User()
            {
                FirstName = "John",
                LastName = "Smith",
                Address = address
            };
                       
            //db.DelPartDoc("{'FirstName':$,'LastName':$}", docID);

            //db.DelPartDoc(user, docID);

            db.AddDocs(new object[] { new {Color = "Red" },
                                      new {Color = "Green"},
                                      new {Color = "Blue" }});

            db.AddDoc("{'FirstName':'John', 'LastName':'Smith', 'Status':'Offline'}");

            //save doc
            uint docId = db.AddDoc(user);

            //load doc
            User u2 = db.GetPartDoc<User>(docId);

            //sometime later ....
            //status was changed in the db
            db.UpdPartDoc("{'Status':'Online'}", docId);

            int[] docIDs = db.GetDocsByAttr("{'FirstName':'John'}");

            user = db.GetPartDoc<User>(user, Load.Create(user.Address, LoadEnum.LoadOnlyTopLevel), docId);


            //sometime later ....
            //update only status in the document
            //do not recreate object, do not reload FirstName and LastName !
            db.GetPartDoc<User>(u2, "{'Status':$}", docId);

        }
    }
}
