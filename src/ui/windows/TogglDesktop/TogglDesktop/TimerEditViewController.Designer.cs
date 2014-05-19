namespace TogglDesktop
{
    partial class TimerEditViewController
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
            this.comboBoxDescription = new System.Windows.Forms.ComboBox();
            this.textBoxDuration = new System.Windows.Forms.TextBox();
            this.buttonStart = new System.Windows.Forms.Button();
            this.labelProject = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // comboBoxDescription
            // 
            this.comboBoxDescription.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxDescription.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxDescription.FormattingEnabled = true;
            this.comboBoxDescription.Location = new System.Drawing.Point(4, 4);
            this.comboBoxDescription.Name = "comboBoxDescription";
            this.comboBoxDescription.Size = new System.Drawing.Size(189, 21);
            this.comboBoxDescription.TabIndex = 0;
            this.comboBoxDescription.Text = "What are you doing?";
            this.comboBoxDescription.SelectedIndexChanged += new System.EventHandler(this.comboBoxDescription_SelectedIndexChanged);
            // 
            // textBoxDuration
            // 
            this.textBoxDuration.Location = new System.Drawing.Point(200, 4);
            this.textBoxDuration.Name = "textBoxDuration";
            this.textBoxDuration.Size = new System.Drawing.Size(31, 20);
            this.textBoxDuration.TabIndex = 1;
            this.textBoxDuration.Text = "0 sec";
            this.textBoxDuration.TextChanged += new System.EventHandler(this.textBoxDuration_TextChanged);
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(238, 4);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(64, 23);
            this.buttonStart.TabIndex = 2;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // labelProject
            // 
            this.labelProject.AutoSize = true;
            this.labelProject.Location = new System.Drawing.Point(4, 32);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(79, 13);
            this.labelProject.TabIndex = 3;
            this.labelProject.Text = "NEW - TOGGL";
            // 
            // TimerEditViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.labelProject);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxDuration);
            this.Controls.Add(this.comboBoxDescription);
            this.Name = "TimerEditViewController";
            this.Size = new System.Drawing.Size(305, 55);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBoxDescription;
        private System.Windows.Forms.TextBox textBoxDuration;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Label labelProject;
    }
}
