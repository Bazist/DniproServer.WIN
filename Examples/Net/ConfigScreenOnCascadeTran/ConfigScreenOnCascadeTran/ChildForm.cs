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
    public partial class ChildForm : Form
    {
        public ChildForm(BigDoc db, uint docID)
        {
            InitializeComponent();

            this.DB = db;
            this.DocID = docID;
            
            DB.BeginTran();

            DB.GetPartDoc<ChildForm>(this, DB_JSON, OBJ_JSON, DocID);
        }

        private BigDoc DB { get; set; }
        private uint DocID { get; set; }
        
        const string DB_JSON = "{'confVal2':$,'confVal3':$}";
        const string OBJ_JSON = "{'tbConfig2':{'Text':$},'tbConfig3':{'Text':$}}";

        private void btOK_Click(object sender, EventArgs e)
        {
            DB.UpdPartDoc<ChildForm>(this, DB_JSON, OBJ_JSON, DocID);

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
            DB.UpdPartDoc<ChildForm>(this, DB_JSON, OBJ_JSON, DocID);

            SubChildForm cf = new SubChildForm(DB, DocID);
            cf.ShowDialog();

            DB.GetPartDoc<ChildForm>(this, DB_JSON, OBJ_JSON, DocID);
        }
    }
}
