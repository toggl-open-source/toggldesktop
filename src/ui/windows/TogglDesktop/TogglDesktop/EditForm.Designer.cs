namespace TogglDesktop
{
    partial class EditForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(EditForm));
            this.CloseButton = new System.Windows.Forms.Button();
            this.labelArrowLeft = new System.Windows.Forms.Label();
            this.labelArrowRight = new System.Windows.Forms.Label();
            this.resizeHandle = new System.Windows.Forms.StatusStrip();
            this.SuspendLayout();
            // 
            // CloseButton
            // 
            this.CloseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.CloseButton.BackColor = System.Drawing.SystemColors.ControlLight;
            this.CloseButton.BackgroundImage = global::TogglDesktop.Properties.Resources.icon_x;
            this.CloseButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.CloseButton.FlatAppearance.BorderSize = 0;
            this.CloseButton.FlatAppearance.MouseDownBackColor = System.Drawing.SystemColors.Control;
            this.CloseButton.FlatAppearance.MouseOverBackColor = System.Drawing.SystemColors.Control;
            this.CloseButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.CloseButton.Location = new System.Drawing.Point(290, 3);
            this.CloseButton.Margin = new System.Windows.Forms.Padding(0);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(20, 20);
            this.CloseButton.TabIndex = 0;
            this.CloseButton.TabStop = false;
            this.CloseButton.UseMnemonic = false;
            this.CloseButton.UseVisualStyleBackColor = false;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // labelArrowLeft
            // 
            this.labelArrowLeft.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.labelArrowLeft.Image = global::TogglDesktop.Properties.Resources.popup_arrow;
            this.labelArrowLeft.Location = new System.Drawing.Point(0, 201);
            this.labelArrowLeft.Name = "labelArrowLeft";
            this.labelArrowLeft.Size = new System.Drawing.Size(13, 24);
            this.labelArrowLeft.TabIndex = 1;
            // 
            // labelArrowRight
            // 
            this.labelArrowRight.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelArrowRight.Image = global::TogglDesktop.Properties.Resources.popup_arrow_right;
            this.labelArrowRight.Location = new System.Drawing.Point(312, 201);
            this.labelArrowRight.Name = "labelArrowRight";
            this.labelArrowRight.Size = new System.Drawing.Size(13, 24);
            this.labelArrowRight.TabIndex = 2;
            // 
            // resizeHandle
            // 
            this.resizeHandle.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.resizeHandle.AutoSize = false;
            this.resizeHandle.BackColor = System.Drawing.Color.WhiteSmoke;
            this.resizeHandle.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.resizeHandle.Dock = System.Windows.Forms.DockStyle.None;
            this.resizeHandle.ImageScalingSize = new System.Drawing.Size(15, 15);
            this.resizeHandle.Location = new System.Drawing.Point(293, 395);
            this.resizeHandle.Name = "resizeHandle";
            this.resizeHandle.Size = new System.Drawing.Size(20, 20);
            this.resizeHandle.TabIndex = 3;
            this.resizeHandle.MouseDown += new System.Windows.Forms.MouseEventHandler(this.resizeHandle_MouseDown);
            this.resizeHandle.MouseMove += new System.Windows.Forms.MouseEventHandler(this.resizeHandle_MouseMove);
            // 
            // EditForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.WhiteSmoke;
            this.BackgroundImage = global::TogglDesktop.Properties.Resources.popup_back_right;
            this.ClientSize = new System.Drawing.Size(325, 420);
            this.ControlBox = false;
            this.Controls.Add(this.resizeHandle);
            this.Controls.Add(this.labelArrowRight);
            this.Controls.Add(this.labelArrowLeft);
            this.Controls.Add(this.CloseButton);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(287, 280);
            this.Name = "EditForm";
            this.Padding = new System.Windows.Forms.Padding(11, 0, 11, 4);
            this.ShowInTaskbar = false;
            this.Text = "EditForm";
            this.TransparencyKey = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button CloseButton;
        private System.Windows.Forms.Label labelArrowLeft;
        private System.Windows.Forms.Label labelArrowRight;
        private System.Windows.Forms.StatusStrip resizeHandle;
    }
}