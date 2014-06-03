namespace TogglDesktop
{
    partial class TimeEntryCellWithHeader
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
            this.labelProject = new System.Windows.Forms.Label();
            this.labelDuration = new System.Windows.Forms.Label();
            this.buttonContinue = new System.Windows.Forms.Button();
            this.labelDescription = new System.Windows.Forms.Label();
            this.labelBillable = new System.Windows.Forms.Label();
            this.labelFormattedDate = new System.Windows.Forms.Label();
            this.labelDateDuration = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // labelProject
            // 
            this.labelProject.AutoSize = true;
            this.labelProject.Location = new System.Drawing.Point(4, 55);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(79, 13);
            this.labelProject.TabIndex = 9;
            this.labelProject.Text = "NEW - TOGGL";
            // 
            // labelDuration
            // 
            this.labelDuration.AutoSize = true;
            this.labelDuration.Location = new System.Drawing.Point(160, 50);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(49, 13);
            this.labelDuration.TabIndex = 8;
            this.labelDuration.Text = "03:21:30";
            // 
            // buttonContinue
            // 
            this.buttonContinue.Location = new System.Drawing.Point(131, 45);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(23, 23);
            this.buttonContinue.TabIndex = 7;
            this.buttonContinue.Text = ">";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // labelDescription
            // 
            this.labelDescription.AutoSize = true;
            this.labelDescription.Location = new System.Drawing.Point(23, 30);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(102, 13);
            this.labelDescription.TabIndex = 6;
            this.labelDescription.Text = "Blogpost about ne...";
            // 
            // labelBillable
            // 
            this.labelBillable.AutoSize = true;
            this.labelBillable.Location = new System.Drawing.Point(4, 30);
            this.labelBillable.Name = "labelBillable";
            this.labelBillable.Size = new System.Drawing.Size(13, 13);
            this.labelBillable.TabIndex = 5;
            this.labelBillable.Text = "$";
            // 
            // labelFormattedDate
            // 
            this.labelFormattedDate.AutoSize = true;
            this.labelFormattedDate.Location = new System.Drawing.Point(10, 6);
            this.labelFormattedDate.Name = "labelFormattedDate";
            this.labelFormattedDate.Size = new System.Drawing.Size(64, 13);
            this.labelFormattedDate.TabIndex = 10;
            this.labelFormattedDate.Text = "Thu 22. Jan";
            // 
            // labelDateDuration
            // 
            this.labelDateDuration.AutoSize = true;
            this.labelDateDuration.Location = new System.Drawing.Point(98, 6);
            this.labelDateDuration.Name = "labelDateDuration";
            this.labelDateDuration.Size = new System.Drawing.Size(56, 13);
            this.labelDateDuration.TabIndex = 11;
            this.labelDateDuration.Text = "2 h 12 min";
            // 
            // TimeEntryCellWithHeader
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.labelDateDuration);
            this.Controls.Add(this.labelFormattedDate);
            this.Controls.Add(this.labelProject);
            this.Controls.Add(this.labelDuration);
            this.Controls.Add(this.buttonContinue);
            this.Controls.Add(this.labelDescription);
            this.Controls.Add(this.labelBillable);
            this.Name = "TimeEntryCellWithHeader";
            this.Size = new System.Drawing.Size(218, 82);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.TimeEntryCellWithHeader_MouseClick);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelProject;
        private System.Windows.Forms.Label labelDuration;
        private System.Windows.Forms.Button buttonContinue;
        private System.Windows.Forms.Label labelDescription;
        private System.Windows.Forms.Label labelBillable;
        private System.Windows.Forms.Label labelFormattedDate;
        private System.Windows.Forms.Label labelDateDuration;
    }
}
