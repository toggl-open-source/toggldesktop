namespace TogglDesktop
{
    partial class TimeEntryCell
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.labelBillable = new System.Windows.Forms.Label();
            this.labelDescription = new System.Windows.Forms.Label();
            this.buttonContinue = new System.Windows.Forms.Button();
            this.labelDuration = new System.Windows.Forms.Label();
            this.labelProject = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // labelBillable
            // 
            this.labelBillable.AutoSize = true;
            this.labelBillable.Location = new System.Drawing.Point(15, 0);
            this.labelBillable.Name = "labelBillable";
            this.labelBillable.Size = new System.Drawing.Size(13, 13);
            this.labelBillable.TabIndex = 0;
            this.labelBillable.Text = "$";
            // 
            // labelDescription
            // 
            this.labelDescription.AutoSize = true;
            this.labelDescription.Location = new System.Drawing.Point(30, 0);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(102, 13);
            this.labelDescription.TabIndex = 1;
            this.labelDescription.Text = "Blogpost about ne...";
            this.labelDescription.Click += new System.EventHandler(this.labelDescription_Click);
            // 
            // buttonContinue
            // 
            this.buttonContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonContinue.Location = new System.Drawing.Point(215, 15);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(23, 23);
            this.buttonContinue.TabIndex = 2;
            this.buttonContinue.Text = ">";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // labelDuration
            // 
            this.labelDuration.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDuration.AutoSize = true;
            this.labelDuration.Location = new System.Drawing.Point(250, 20);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(49, 13);
            this.labelDuration.TabIndex = 3;
            this.labelDuration.Text = "03:21:30";
            this.labelDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDuration.Click += new System.EventHandler(this.labelDuration_Click);
            // 
            // labelProject
            // 
            this.labelProject.AutoSize = true;
            this.labelProject.Location = new System.Drawing.Point(15, 25);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(79, 13);
            this.labelProject.TabIndex = 4;
            this.labelProject.Text = "NEW - TOGGL";
            this.labelProject.Click += new System.EventHandler(this.labelProject_Click);
            // 
            // TimeEntryCell
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.labelProject);
            this.Controls.Add(this.labelDuration);
            this.Controls.Add(this.buttonContinue);
            this.Controls.Add(this.labelDescription);
            this.Controls.Add(this.labelBillable);
            this.Name = "TimeEntryCell";
            this.Size = new System.Drawing.Size(314, 49);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.TimeEntryCell_MouseClick);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelBillable;
        private System.Windows.Forms.Label labelDescription;
        private System.Windows.Forms.Button buttonContinue;
        private System.Windows.Forms.Label labelDuration;
        private System.Windows.Forms.Label labelProject;
    }
}
