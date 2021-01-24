namespace ConfigScreenOnCascadeTran
{
    partial class SubChildForm
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
            this.btOK = new System.Windows.Forms.Button();
            this.btCancel = new System.Windows.Forms.Button();
            this.tbConfig3 = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.tbConfig4 = new System.Windows.Forms.TextBox();
            this.btEdit = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btOK
            // 
            this.btOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btOK.Location = new System.Drawing.Point(171, 85);
            this.btOK.Name = "btOK";
            this.btOK.Size = new System.Drawing.Size(75, 23);
            this.btOK.TabIndex = 0;
            this.btOK.Text = "OK";
            this.btOK.UseVisualStyleBackColor = true;
            this.btOK.Click += new System.EventHandler(this.btOK_Click);
            // 
            // btCancel
            // 
            this.btCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btCancel.Location = new System.Drawing.Point(252, 85);
            this.btCancel.Name = "btCancel";
            this.btCancel.Size = new System.Drawing.Size(75, 23);
            this.btCancel.TabIndex = 1;
            this.btCancel.Text = "Cancel";
            this.btCancel.UseVisualStyleBackColor = true;
            this.btCancel.Click += new System.EventHandler(this.btCancel_Click);
            // 
            // tbConfig3
            // 
            this.tbConfig3.Location = new System.Drawing.Point(93, 19);
            this.tbConfig3.Name = "tbConfig3";
            this.tbConfig3.Size = new System.Drawing.Size(234, 20);
            this.tbConfig3.TabIndex = 2;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(2, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(85, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Config Setting 3:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(2, 48);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(85, 13);
            this.label2.TabIndex = 9;
            this.label2.Text = "Config Setting 4:";
            // 
            // tbConfig4
            // 
            this.tbConfig4.Enabled = false;
            this.tbConfig4.Location = new System.Drawing.Point(93, 45);
            this.tbConfig4.Name = "tbConfig4";
            this.tbConfig4.Size = new System.Drawing.Size(153, 20);
            this.tbConfig4.TabIndex = 8;
            // 
            // btEdit
            // 
            this.btEdit.Location = new System.Drawing.Point(252, 43);
            this.btEdit.Name = "btEdit";
            this.btEdit.Size = new System.Drawing.Size(75, 23);
            this.btEdit.TabIndex = 7;
            this.btEdit.Text = "Edit";
            this.btEdit.UseVisualStyleBackColor = true;
            // 
            // SubChildForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(339, 117);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.tbConfig4);
            this.Controls.Add(this.btEdit);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbConfig3);
            this.Controls.Add(this.btCancel);
            this.Controls.Add(this.btOK);
            this.Name = "SubChildForm";
            this.Text = "Sub Child Config Screen";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btOK;
        private System.Windows.Forms.Button btCancel;
        private System.Windows.Forms.TextBox tbConfig3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbConfig4;
        private System.Windows.Forms.Button btEdit;
        private System.Windows.Forms.Label label2;
    }
}

