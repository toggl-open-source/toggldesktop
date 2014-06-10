namespace TogglDesktop
{
    partial class FeedbackWindowController
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FeedbackWindowController));
            this.label1 = new System.Windows.Forms.Label();
            this.comboBoxTopic = new System.Windows.Forms.ComboBox();
            this.richTextBoxContents = new System.Windows.Forms.RichTextBox();
            this.buttonUploadImage = new System.Windows.Forms.Button();
            this.buttonSend = new System.Windows.Forms.Button();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(186, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Send us your feedback and questions";
            // 
            // comboBoxTopic
            // 
            this.comboBoxTopic.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTopic.FormattingEnabled = true;
            this.comboBoxTopic.Items.AddRange(new object[] {
            "- Please specify a topic -",
            "Bug report",
            "Feature request",
            "Other"});
            this.comboBoxTopic.Location = new System.Drawing.Point(15, 25);
            this.comboBoxTopic.Name = "comboBoxTopic";
            this.comboBoxTopic.Size = new System.Drawing.Size(257, 21);
            this.comboBoxTopic.TabIndex = 1;
            // 
            // richTextBoxContents
            // 
            this.richTextBoxContents.Location = new System.Drawing.Point(15, 52);
            this.richTextBoxContents.Name = "richTextBoxContents";
            this.richTextBoxContents.Size = new System.Drawing.Size(257, 155);
            this.richTextBoxContents.TabIndex = 2;
            this.richTextBoxContents.Text = "";
            // 
            // buttonUploadImage
            // 
            this.buttonUploadImage.Location = new System.Drawing.Point(190, 213);
            this.buttonUploadImage.Name = "buttonUploadImage";
            this.buttonUploadImage.Size = new System.Drawing.Size(82, 23);
            this.buttonUploadImage.TabIndex = 3;
            this.buttonUploadImage.Text = "Upload image";
            this.buttonUploadImage.UseVisualStyleBackColor = true;
            this.buttonUploadImage.Click += new System.EventHandler(this.buttonUploadImage_Click);
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(190, 242);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(82, 23);
            this.buttonSend.TabIndex = 4;
            this.buttonSend.Text = "Send";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // openFileDialog
            // 
            this.openFileDialog.Title = "Select image for upload";
            // 
            // FeedbackWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 278);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.buttonUploadImage);
            this.Controls.Add(this.richTextBoxContents);
            this.Controls.Add(this.comboBoxTopic);
            this.Controls.Add(this.label1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "FeedbackWindowController";
            this.Text = "Send Feedback";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FeedbackWindowController_FormClosing);
            this.Load += new System.EventHandler(this.FeedbackWindowController_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxTopic;
        private System.Windows.Forms.RichTextBox richTextBoxContents;
        private System.Windows.Forms.Button buttonUploadImage;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
    }
}