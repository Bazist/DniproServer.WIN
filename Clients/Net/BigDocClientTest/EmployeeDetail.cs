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

using BigDocClient;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BigDocClientTest
{
    public partial class EmployeeDetail : Form
    {
        public EmployeeDetail(string company)
        {
            InitializeComponent();

            Childs.Rows.Clear();
            Childs.Columns.Clear();
            Childs.Columns.Add("FirstName", typeof(string));
            Childs.Columns.Add("LastName", typeof(string));

            _company = company;
        }

        private string _company;

        BigDoc db = new BigDoc("booben.com", 4477);

        public string FirstName
        {
            set
            {
                tbFirstName.Text = value;
            }

            get
            {
                return tbFirstName.Text;
            }
        }

        public string LastName
        {
            set
            {
                tbLastName.Text = value;
            }

            get
            {
                return tbLastName.Text;
            }
        }

        public string Age
        {
            set
            {
                tbAge.Text = value;
            }

            get
            {
                return tbAge.Text;
            }
        }

        public DataTable Childs = new DataTable();
        private string _oldFirstName;
        private string _oldLastName;

        private void DetailForm_Load(object sender, EventArgs e)
        {
            dgChilds.DataSource = Childs;

            _oldFirstName = FirstName;
            _oldLastName = LastName;
        }

        private void dgChilds_DoubleClick(object sender, EventArgs e)
        {
            var row = dgChilds.SelectedRows[0];

            string query = "{'Company':'" + _company + "', 'Employees':[{'FirstName':'" + tbFirstName.Text + "','LastName':'" + tbLastName.Text + "','Childs':[{'FirstName':'" + row.Cells[0].Value + "','LastName':'" + row.Cells[1].Value + "'}]}]}";

            EmployeeChildDetail df = new EmployeeChildDetail(_company, tbFirstName.Text, tbLastName.Text);

            //db.GetWhereElems(query).Select<EmployeeChildDetail>(df, "{'Employees':[{'Childs':[!{'FirstName':$,'LastName':$,'Age':$}]}]}", false, new Serialization.DataTableBehaviour());

            df.ShowDialog();
        }

        private void btSave_Click(object sender, EventArgs e)
        {
            string query = "{'Company':'" + _company + "', 'Employees':[{'FirstName':'" + _oldFirstName + "','LastName':'" + _oldLastName + "'}]}";

            //db.GetWhereElems(query).Update<EmployeeDetail>(this, "{'Employees':[!{'FirstName':$, 'LastName':$, 'Age':$}]}");

            this.Close();

        }

        private void btAdd_Click(object sender, EventArgs e)
        {
            EmployeeChildDetail ecd = new EmployeeChildDetail(_company, tbFirstName.Text, tbLastName.Text);

            if (ecd.ShowDialog() == DialogResult.OK)
            {
                string query = "{'Company':'" + _company + "', 'Employees':[{'FirstName':'" + _oldFirstName + "','LastName':'" + _oldLastName + "'}]}";

                //db.GetWhereElems(query).Insert<EmployeeChildDetail>(ecd, "{'Employees':[{'Childs':[Add,!{'FirstName':$,'LastName':$,'Age':$}]}]}");
            }
        }
    }
}
