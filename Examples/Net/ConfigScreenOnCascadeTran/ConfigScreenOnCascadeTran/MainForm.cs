using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using BigDocClient;

namespace ConfigScreenOnCascadeTran
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            DB = new BigDoc("localhost", 4477);

            DB.BeginTran();

            LoadDoc();
        }

        private BigDoc DB { get; set; }
        private uint DocID { get; set; }
        
        const string DB_HEADER_JSON = "{'type':'config'}";
        const string DB_JSON = "{'confVal1':$,'confVal2':$}";
        const string OBJ_JSON = "{'tbConfig1':{'Text':$},'tbConfig2':{'Text':$}}";

        private void LoadDoc()
        {
            //if doesn't exists, create
            DocID = DB.GetWhere(DB_HEADER_JSON).GetFirstID();

            if (DocID == 0)
            {
                DocID = DB.AddDoc(DB_HEADER_JSON);
            }
            else
            {
                DB.GetPartDoc<MainForm>(this, DB_JSON, OBJ_JSON, DocID);
            }
        }

        private void btOK_Click(object sender, EventArgs e)
        {
            DB.UpdPartDoc<MainForm>(this, DB_JSON, OBJ_JSON, DocID);

            DB.CommitTran();

            this.Close();
        }

        private void btCancel_Click(object sender, EventArgs e)
        {
            DB.RollbackTran();

            this.Close();
        }

        private void btEdit_Click(object sender, EventArgs e)
        {
            DB.UpdPartDoc<MainForm>(this, DB_JSON, OBJ_JSON, DocID);

            ChildForm cf = new ChildForm(DB, DocID);
            cf.ShowDialog();

            LoadDoc();
        }
    }
}
