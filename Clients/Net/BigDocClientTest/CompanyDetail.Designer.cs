namespace DniproClientTest
{
    partial class CompanyDetail
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.tbCompany = new System.Windows.Forms.TextBox();
            this.dgMainForm = new System.Windows.Forms.DataGridView();
            this.tbCountry = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.btSave = new System.Windows.Forms.Button();
            this.btOK = new System.Windows.Forms.Button();
            this.btAdd = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.dgMainForm)).BeginInit();
            this.SuspendLayout();
            // 
            // tbCompany
            // 
            this.tbCompany.Location = new System.Drawing.Point(92, 15);
            this.tbCompany.Name = "tbCompany";
            this.tbCompany.Size = new System.Drawing.Size(169, 20);
            this.tbCompany.TabIndex = 2;
            // 
            // dgMainForm
            // 
            this.dgMainForm.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgMainForm.Location = new System.Drawing.Point(21, 92);
            this.dgMainForm.MultiSelect = false;
            this.dgMainForm.Name = "dgMainForm";
            this.dgMainForm.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dgMainForm.Size = new System.Drawing.Size(240, 117);
            this.dgMainForm.TabIndex = 4;
            this.dgMainForm.DoubleClick += new System.EventHandler(this.dgMainForm_DoubleClick);
            // 
            // tbCountry
            // 
            this.tbCountry.Location = new System.Drawing.Point(92, 53);
            this.tbCountry.Name = "tbCountry";
            this.tbCountry.Size = new System.Drawing.Size(169, 20);
            this.tbCountry.TabIndex = 5;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(18, 18);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(51, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Company";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(21, 59);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(43, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "Country";
            // 
            // btSave
            // 
            this.btSave.Location = new System.Drawing.Point(186, 260);
            this.btSave.Name = "btSave";
            this.btSave.Size = new System.Drawing.Size(75, 23);
            this.btSave.TabIndex = 8;
            this.btSave.Text = "Save";
            this.btSave.UseVisualStyleBackColor = true;
            this.btSave.Click += new System.EventHandler(this.btSave_Click);
            // 
            // btOK
            // 
            this.btOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btOK.Location = new System.Drawing.Point(24, 260);
            this.btOK.Name = "btOK";
            this.btOK.Size = new System.Drawing.Size(75, 23);
            this.btOK.TabIndex = 9;
            this.btOK.Text = "OK";
            this.btOK.UseVisualStyleBackColor = true;
            // 
            // btAdd
            // 
            this.btAdd.Location = new System.Drawing.Point(186, 215);
            this.btAdd.Name = "btAdd";
            this.btAdd.Size = new System.Drawing.Size(75, 23);
            this.btAdd.TabIndex = 10;
            this.btAdd.Text = "Add";
            this.btAdd.UseVisualStyleBackColor = true;
            this.btAdd.Click += new System.EventHandler(this.btAdd_Click);
            // 
            // CompanyDetail
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 295);
            this.Controls.Add(this.btAdd);
            this.Controls.Add(this.btOK);
            this.Controls.Add(this.btSave);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbCountry);
            this.Controls.Add(this.dgMainForm);
            this.Controls.Add(this.tbCompany);
            this.Name = "CompanyDetail";
            this.Text = "Company Detail";
            this.Load += new System.EventHandler(this.CompanyDetail_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dgMainForm)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.TextBox tbCompany;
        private System.Windows.Forms.DataGridView dgMainForm;
        private System.Windows.Forms.TextBox tbCountry;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btSave;
        private System.Windows.Forms.Button btOK;
        private System.Windows.Forms.Button btAdd;
    }
}

