using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using DniproClient;

namespace ConfigScreenOnCascadeTran
{
    public partial class SubChildForm : Form
    {
        public SubChildForm(DniproDB db, uint docID)
        {
            InitializeComponent();

            this.DB = db;
            this.DocID = docID;
            
            DB.BeginTran();

            DB.GetPartDoc<SubChildForm>(this, DB_JSON, OBJ_JSON, DocID);
        }

        private DniproDB DB { get; set; }
        private uint DocID { get; set; }
        
        const string DB_JSON = "{'confVal3':$,'confVal4':$}";
        const string OBJ_JSON = "{'tbConfig3':{'Text':$},'tbConfig4':{'Text':$}}";

        private void btOK_Click(object sender, EventArgs e)
        {
            DB.UpdPartDoc<SubChildForm>(this, DB_JSON, OBJ_JSON, DocID);

            DB.CommitTran();

            this.Close();
        }

        private void btCancel_Click(object sender, EventArgs e)
        {
            DB.RollbackTran();

            this.Close();
        }
    }
}
