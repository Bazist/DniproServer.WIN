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
using System.Text;
using System.Windows.Forms;
using BigDocClient;

namespace BigDocClientTest
{
    public partial class CompanyDetail : Form
    {
        public CompanyDetail()
        {
            InitializeComponent();

            Employees.Columns.Clear();
            Employees.Columns.Add("FirstName", typeof(string));
            Employees.Columns.Add("LastName", typeof(string));
        }

        BigDoc db = new BigDoc("booben.com", 4477);

        public string Company
        {
            set
            {
                tbCompany.Text = value;
            }

            get
            {
                return tbCompany.Text;
            }
        }

        public string Country
        {
            set
            {
                tbCountry.Text = value;
            }

            get
            {
                return tbCountry.Text;
            }
        }

        public DataTable Employees = new DataTable();

        private void dgMainForm_DoubleClick(object sender, EventArgs e)
        {
            var row = dgMainForm.SelectedRows[0];

            string query = "{'Company':'" + tbCompany.Text + "','Employees':[{'FirstName':'" + row.Cells[0].Value + "','LastName':'" + row.Cells[1].Value + "'}]}";

            EmployeeDetail df = new EmployeeDetail(tbCompany.Text);

            //db.GetWhereElems(query).Select<EmployeeDetail>(df, "{'Employees':[!{'FirstName':$,'LastName':$,'Age':$,'Childs':[R,{'FirstName':$,'LastName':$}]}]}", false, new Serialization.DataTableBehaviour());

            df.ShowDialog();
        }

        private string _oldCompany;

        private void CompanyDetail_Load(object sender, EventArgs e)
        {
            dgMainForm.DataSource = Employees;

            _oldCompany = tbCompany.Text;
        }

        private void btSave_Click(object sender, EventArgs e)
        {
            string query = "{'Company':'" + _oldCompany + "'}";

            db.GetWhere(query).Update<CompanyDetail>(this, "{'Company':$, 'Country':$}");

            this.Close();
        }

        private void btAdd_Click(object sender, EventArgs e)
        {
            EmployeeDetail ed = new EmployeeDetail(tbCompany.Text);

            if (ed.ShowDialog() == DialogResult.OK)
            {
                string query = "{'Company':'" + tbCompany.Text + "'}";

                db.GetWhere(query).Insert<EmployeeDetail>(ed, "{'Employees':[Add,!{'FirstName':$,'LastName':$,'Age':$}]}");
            }
        }
    }
}
