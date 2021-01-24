/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: dniprodb@gmail.com, skype: vyacheslavm81)
# This file is part of DniproClient.
#
# DniproClient is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# DniproClient is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

using DniproClient;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace DniproClientTest
{
    public partial class EmployeeChildDetail : Form
    {
        public EmployeeChildDetail(string company, string parentFirstName, string parentLastName)
        {
            InitializeComponent();

            _company = company;
            _parentFirstName = parentFirstName;
            _parentLastName = parentLastName;
        }

        private string _company;
        private string _parentFirstName;
        private string _parentLastName;
        private string _oldFirstName;
        private string _oldLastName;

        DniproDB db = new DniproDB("booben.com", 4477);

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

        private void btSave_Click(object sender, EventArgs e)
        {
            string query = "{'Company':'" + _company + "', 'Employees':[{'FirstName':'" + _parentFirstName + "','LastName':'" + _parentLastName + "','Childs':[{'FirstName':'" + _oldFirstName + "','LastName':'" + _oldLastName + "'}]}]}";

            //db.GetWhereElems(query).Update<EmployeeChildDetail>(this, "{'Employees':[{'Childs':[!{'FirstName':$,'LastName':$,'Age':$}]}]}");

            this.Close();
        }

        private void EmployeeChildDetail_Load(object sender, EventArgs e)
        {
            _oldFirstName = tbFirstName.Text;
            _oldLastName = tbLastName.Text;
        }
    }
}
