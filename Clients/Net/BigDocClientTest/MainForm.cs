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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Windows.Forms;
using BigDocClient;

namespace BigDocClientTest
{
    public partial class MainForm : Form
    {
        //db.LoadPartDoc(this, "{'textBox1':{'Text':$},'comboBox1':{'Items':[1,2,3],'SelectedIndex':$}}", 1);

        public MainForm()
        {
            InitializeComponent();
        }

        BigDoc db = new BigDoc("127.0.0.1", 4477);

        public DataTable Companies = new DataTable();

        private void button1_Click(object sender, EventArgs e)
        {
            if (db.GetAll().Count() == 0)
            {
                db.AddDoc("{'Company':'AnySoft', 'Country':'USA', 'Employees':[{'FirstName':'Ivan','LastName':'Ivanov','Age':20, 'Childs':[{'FirstName':'Ivanko','LastName':'Ivankov','Age':2},{'FirstName':'Ivanka','LastName':'Ivankova','Age':3}]}," +
                                                                              "{'FirstName':'Petr','LastName':'Petrov','Age':21, 'Childs':[{'FirstName':'Petrako','LastName':'Petravkov','Age':1},{'FirstName':'Petravka','LastName':'Petravkova','Age':5}]}]}");

                db.AddDoc("{'Company':'MySoft', 'Country':'Ukraine', 'Employees':[{'FirstName':'Иван','LastName':'Иванов','Age':20, 'Childs':[{'FirstName':'Иванко','LastName':'Иванков','Age':2},{'FirstName':'Иванка','LastName':'Иванкова','Age':3}]}," +
                                                                                 "{'FirstName':'Петр','LastName':'Петров','Age':21, 'Childs':[{'FirstName':'Петрако','LastName':'Петраков','Age':1},{'FirstName':'Петравка','LastName':'Петравкова','Age':5}]}]}");
            }

            Companies.Rows.Clear();
            Companies.Columns.Clear();
            Companies.Columns.Add("Company", typeof(string));
            Companies.Columns.Add("Country", typeof(string));

            //load grid
            //db.GetAll().Select<DataTable>(Companies, "{'Company':$,'Country':$}", false, new Serialization.DataTableBehaviour());

            dgCompany.DataSource = Companies;
        }

        private void dgMainForm_DoubleClick(object sender, EventArgs e)
        {
            var row = dgCompany.SelectedRows[0];

            CompanyDetail cd = new CompanyDetail();

            string query = "{'Company':'" + row.Cells[0].Value + "','Country':'" + row.Cells[1].Value + "'}";

            //db.GetWhere(query).Select<CompanyDetail>(cd, "{'Company':$, 'Country':$, 'Employees':[R, {'FirstName':$, 'LastName':$}]}", false, new Serialization.DataTableBehaviour());

            cd.ShowDialog();
        }

        private void btAdd_Click(object sender, EventArgs e)
        {
            CompanyDetail cd = new CompanyDetail();

            if (cd.ShowDialog() == DialogResult.OK)
            {
                db.AddDoc<CompanyDetail>(cd, "{'Company':$,'Country':$}");
            }
        }

        class Employee
        {
            public byte[] blob {get; set;}
        };

        private void Func(Expression<Func<Employee, object>> g)
        {
            
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Func(x => x.blob);


            //db.AddColl("Employee");

            //db["Employee"].AddDoc("{'attr':'val'}");
            //db["Employee"].AddDoc("{'attr':'val'}");

            //uint em = db["Employee"].GetAll().Count();

            //db.AddDoc(new { blob = new byte[1024] }, "{'blob':$}");
            

            //DateTime now = DateTime.Now;

            //db.BeginTran();

            //for (int i = 0; i < 100000; i++)
            //{
            //    db.AddDoc("{'attr':'val'}");
            //}

            //db.CommitTran();

            //MessageBox.Show(DateTime.Now.Subtract(now).TotalMilliseconds.ToString());

            /*
            db.AddDoc("{'type':'employee', 'name':'Mykola', 'carModel':'zaz'}"); //документ описывающий параметры сотрудника
            db.AddDoc("{'type':'car', 'model':'zaz', 'country':'ua', 'engine':'1.2'}"); //документ описывающий параметры автомобиля

            Employee[] ems = db.GetWhere("{'type':'employee', 'name':'Mykola'}"). //получаем документ где описаны параметры сотрудника
                                Join("{'carModel':$}", "{'type':'car', 'model':$}"). //джоиним к сотруднику параметры его автомобиля
                                Select<Employee>("{'name':$}{'engine':$}"); //выводим им

            //db.AddDoc("{'arr':[1,2,[3,4,[5,6],7],8]}");
            //db.AddDoc("{'arr':[4,6,[7,4,[7,6],9],0]}");
            //db.AddDoc("{'arr':[1,2,8]}");
            //db.AddDoc("{'arr':[1,2,[3,4,[5,[9,3],6],7],8]}");
            
            //...

            //uint count = db.GetWhereElems("{'arr':[[[7]]]}").Count(); //~200-300 nanoseconds
            

            //Serialization.DeserializeProxy<MenuStrip>(this.menuStrip1, "{'Items':[{'Text':'Open'}]}");

            //add doc to DB
            //db.AddDoc(new
            //{
            //    Company = "AnySoft",
            //    Country = "UK",

            //    Employees = new[]
            //    {
            //        new { FirstName = "Ivan",
            //              LastName = "Ivanov",
            //              Age = 25,
            //              Childs = new []
            //              {
            //                  new
            //                  {
            //                      FirstName = "Ivanka",
            //                      LastName = "Ivankova",
            //                      Age = 5
            //                  }
            //              }
            //        }
            //    }
            //});

            ////create table
            //DataTable dtChilds = new DataTable("Childs");
            //dtChilds.Columns.Add("FirstName", typeof(string));
            //dtChilds.Columns.Add("LastName", typeof(string));



            object obj1;
            object obj2;

            var aBigDoc = new
            {
                A = 1,
                B = obj1 = new
                {
                    x = 1,
                    y = 2
                },
                C = 3,
                D = obj2 = new
                {
                    x = 1,
                    y = 2
                }
            };

            uint docId = 123;

            db.UpdPartDoc(aBigDoc, new[] { Change.Create(obj1, ChangeEnum.UpdateRecursively),
                                           Change.Create(obj2, ChangeEnum.UpdateRecursively) }, docId);

            //load all childs with 5 years age to this table
            //uint count = db.GetWhereElems("{'Country':'UK','Employees':[{'Age':25, 'Childs':[{'Age':5}]}]}").Count();

            //SELECT count(*)
            //FROM Company c
            //    INNER JOIN Employee e ON c.ID = e.CompanyID
            //    INNER JOIN Child h ON e.ID = h.EmployeeID
            //WHERE c.Country = 'UK'
            //    AND e.Age = 25
            //    AND h.Age = 5

            //  .Select<DataTable>(dtChilds, "{'Employees':[{'Childs':[!{'FirstName':$,'LastName':$}]}]}", 0, false, new Serialization.DataTableBehaviour());

            //show table in GUI
            //dgChilds.DataSource = dtChilds;
            */
        }
    }
}
