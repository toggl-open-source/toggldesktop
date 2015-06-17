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
            this.resizeHandle = new System.Windows.Forms.StatusStrip();
            this.SuspendLayout();
            // 
            // resizeHandle
            // 
            this.resizeHandle.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.resizeHandle.AutoSize = false;
            this.resizeHandle.BackColor = System.Drawing.Color.WhiteSmoke;
            this.resizeHandle.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.resizeHandle.Dock = System.Windows.Forms.DockStyle.None;
            this.resizeHandle.ImageScalingSize = new System.Drawing.Size(15, 15);
            this.resizeHandle.Location = new System.Drawing.Point(302, 399);
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
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(287, 280);
            this.Name = "EditForm";
            this.ShowInTaskbar = false;
            this.Text = "EditForm";
            this.TransparencyKey = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.StatusStrip resizeHandle;
}
}