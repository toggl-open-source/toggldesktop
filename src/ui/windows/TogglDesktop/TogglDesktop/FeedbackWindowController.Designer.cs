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
            this.panelMain = new System.Windows.Forms.Panel();
            this.topicreminder = new System.Windows.Forms.Label();
            this.fileNameLabel = new System.Windows.Forms.Label();
            this.panelMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.label1.Location = new System.Drawing.Point(12, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(207, 15);
            this.label1.TabIndex = 0;
            this.label1.Text = "Send us your feedback and questions";
            // 
            // comboBoxTopic
            // 
            this.comboBoxTopic.AccessibleName = "Feedback topic";
            this.comboBoxTopic.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxTopic.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTopic.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.comboBoxTopic.FormattingEnabled = true;
            this.comboBoxTopic.Items.AddRange(new object[] {
            "- Please specify a topic -",
            "Bug report",
            "Feature request",
            "Other"});
            this.comboBoxTopic.Location = new System.Drawing.Point(15, 40);
            this.comboBoxTopic.Name = "comboBoxTopic";
            this.comboBoxTopic.Size = new System.Drawing.Size(270, 23);
            this.comboBoxTopic.TabIndex = 1;
            this.comboBoxTopic.SelectedIndexChanged += new System.EventHandler(this.comboBoxTopic_SelectedIndexChanged);
            // 
            // richTextBoxContents
            // 
            this.richTextBoxContents.AccessibleName = "Feedback contents";
            this.richTextBoxContents.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxContents.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.richTextBoxContents.Location = new System.Drawing.Point(15, 73);
            this.richTextBoxContents.Name = "richTextBoxContents";
            this.richTextBoxContents.Size = new System.Drawing.Size(270, 136);
            this.richTextBoxContents.TabIndex = 2;
            this.richTextBoxContents.Text = "";
            // 
            // buttonUploadImage
            // 
            this.buttonUploadImage.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonUploadImage.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.buttonUploadImage.Location = new System.Drawing.Point(194, 215);
            this.buttonUploadImage.Name = "buttonUploadImage";
            this.buttonUploadImage.Size = new System.Drawing.Size(91, 23);
            this.buttonUploadImage.TabIndex = 3;
            this.buttonUploadImage.Text = "Upload image";
            this.buttonUploadImage.UseVisualStyleBackColor = true;
            this.buttonUploadImage.Click += new System.EventHandler(this.buttonUploadImage_Click);
            // 
            // buttonSend
            // 
            this.buttonSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSend.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.buttonSend.Location = new System.Drawing.Point(194, 244);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(91, 23);
            this.buttonSend.TabIndex = 4;
            this.buttonSend.Text = "Send";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // openFileDialog
            // 
            this.openFileDialog.Title = "Select image for upload";
            // 
            // panelMain
            // 
            this.panelMain.Controls.Add(this.topicreminder);
            this.panelMain.Controls.Add(this.fileNameLabel);
            this.panelMain.Controls.Add(this.label1);
            this.panelMain.Controls.Add(this.buttonSend);
            this.panelMain.Controls.Add(this.comboBoxTopic);
            this.panelMain.Controls.Add(this.buttonUploadImage);
            this.panelMain.Controls.Add(this.richTextBoxContents);
            this.panelMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelMain.Location = new System.Drawing.Point(0, 0);
            this.panelMain.Name = "panelMain";
            this.panelMain.Size = new System.Drawing.Size(300, 286);
            this.panelMain.TabIndex = 5;
            // 
            // topicreminder
            // 
            this.topicreminder.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.topicreminder.AutoSize = true;
            this.topicreminder.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.topicreminder.Location = new System.Drawing.Point(15, 238);
            this.topicreminder.Name = "topicreminder";
            this.topicreminder.Size = new System.Drawing.Size(158, 15);
            this.topicreminder.TabIndex = 6;
            this.topicreminder.Text = "Please select feedback topic!";
            this.topicreminder.Visible = false;
            // 
            // fileNameLabel
            // 
            this.fileNameLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.fileNameLabel.AutoEllipsis = true;
            this.fileNameLabel.Location = new System.Drawing.Point(15, 216);
            this.fileNameLabel.Name = "fileNameLabel";
            this.fileNameLabel.Size = new System.Drawing.Size(173, 23);
            this.fileNameLabel.TabIndex = 5;
            this.fileNameLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // FeedbackWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(300, 286);
            this.Controls.Add(this.panelMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(300, 316);
            this.Name = "FeedbackWindowController";
            this.Text = "Send Feedback";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FeedbackWindowController_FormClosing);
            this.Load += new System.EventHandler(this.FeedbackWindowController_Load);
            this.panelMain.ResumeLayout(false);
            this.panelMain.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxTopic;
        private System.Windows.Forms.RichTextBox richTextBoxContents;
        private System.Windows.Forms.Button buttonUploadImage;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.Panel panelMain;
        private System.Windows.Forms.Label fileNameLabel;
        private System.Windows.Forms.Label topicreminder;
    }
}