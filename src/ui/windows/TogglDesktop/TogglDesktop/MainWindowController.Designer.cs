namespace TogglDesktop
{
    partial class MainWindowController
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
            this.troubleBox = new System.Windows.Forms.Panel();
            this.buttonDismissError = new System.Windows.Forms.Button();
            this.errorLabel = new System.Windows.Forms.Label();
            this.troubleBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // troubleBox
            // 
            this.troubleBox.Controls.Add(this.buttonDismissError);
            this.troubleBox.Controls.Add(this.errorLabel);
            this.troubleBox.Location = new System.Drawing.Point(0, 0);
            this.troubleBox.Name = "troubleBox";
            this.troubleBox.Size = new System.Drawing.Size(276, 30);
            this.troubleBox.TabIndex = 5;
            this.troubleBox.Visible = false;
            // 
            // buttonDismissError
            // 
            this.buttonDismissError.Location = new System.Drawing.Point(3, 3);
            this.buttonDismissError.Name = "buttonDismissError";
            this.buttonDismissError.Size = new System.Drawing.Size(16, 23);
            this.buttonDismissError.TabIndex = 6;
            this.buttonDismissError.Text = "X";
            this.buttonDismissError.UseVisualStyleBackColor = true;
            this.buttonDismissError.Click += new System.EventHandler(this.buttonDismissError_Click);
            // 
            // errorLabel
            // 
            this.errorLabel.AutoSize = true;
            this.errorLabel.Location = new System.Drawing.Point(25, 8);
            this.errorLabel.Name = "errorLabel";
            this.errorLabel.Size = new System.Drawing.Size(64, 13);
            this.errorLabel.TabIndex = 5;
            this.errorLabel.Text = "Login failed!";
            // 
            // MainWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(277, 292);
            this.Controls.Add(this.troubleBox);
            this.Name = "MainWindowController";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Toggl Desktop";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainWindowController_FormClosing);
            this.Load += new System.EventHandler(this.MainWindowController_Load);
            this.troubleBox.ResumeLayout(false);
            this.troubleBox.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel troubleBox;
        private System.Windows.Forms.Label errorLabel;
        private System.Windows.Forms.Button buttonDismissError;
    }
}

