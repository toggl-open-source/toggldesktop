using System;
using System.Drawing;
using System.Windows.Controls;
using System.Windows.Forms;
using System.Windows.Input;
using Cursors = System.Windows.Forms.Cursors;
using MouseEventArgs = System.Windows.Input.MouseEventArgs;

namespace TogglDesktop
{
public partial class EditForm : Form
{
    private bool isResizing;
    private bool isLeft;

    public string GUID = null;
    private WPF.TimeEntryEditViewController controller;
    private bool remainOnTop;

    public EditForm()
    {
        InitializeComponent();
        Padding = new Padding(0, 0, 0, 0);
    }

    public void SetPlacement(bool left, Point p, int height, bool fixHeight = false)
    {
        this.controller.SetShadow(left, height);

        if(!fixHeight)
            height = Math.Min(700, Math.Max(450, height));

        this.TopMost = true;
        this.isLeft = !left;
        this.Height = height;


        if (left)
        {
            p.X -= this.Width;
        }

        this.Location = p;
        this.MinimumSize = new Size(400, height);
        this.MaximumSize = new Size(1200, height);
    }

    internal void reset()
    {
        // TODO: what was this doing before?
    }

    protected override void OnDeactivate(EventArgs e)
    {
        base.OnDeactivate(e);
        this.SetWindowPos(this.remainOnTop);
    }

    public void SetWindowPos(bool onTop)
    {
        this.remainOnTop = onTop;
        // FIXME: this seems to hang now:
        // Win32.SetWindowPos(this.Handle, onTop ? Win32.HWND_TOPMOST : Win32.HWND_NOTOPMOST, 0, 0, 0, 0, Win32.SWP_NOMOVE | Win32.SWP_NOSIZE);
    }

    private void mouseDown(MouseEventArgs e)
    {
        if (e.LeftButton != MouseButtonState.Pressed)
            return;

        this.isResizing = this.isMouseOverResizeArea(e);
    }

    private void mouseMove(MouseEventArgs e)
    {
        if (this.isResizing || this.isMouseOverResizeArea(e))
        {
            this.Cursor = Cursors.SizeWE;
        }
        else if(this.isMouseOverEmptyArea())
        {
            this.Cursor = Cursors.Arrow;
        }

        if (!this.isResizing)
        {
            return;
        }
        this.isResizing = e.LeftButton == MouseButtonState.Pressed;
        Win32.ReleaseCapture();

        const int htleft = 10;
        const int htright = 11;

        Win32.SendMessage(this.Handle,
                          this.isResizing ? Win32.wmNcLButtonDown : Win32.wmNcLButtonUp,
                          this.isLeft ? htright : htleft,
                          0);
    }

    protected override void OnResize(EventArgs e)
    {
        if (this.WindowState == FormWindowState.Maximized)
        {
            this.WindowState = FormWindowState.Normal;
        }

        base.OnResize(e);
    }

    private bool isMouseOverResizeArea(MouseEventArgs e)
    {
        var mousePosition = e.GetPosition(this.controller);

        const int resizeAreaWidth = 10;

        return this.isLeft
               ? mousePosition.X > this.controller.ActualWidth - resizeAreaWidth
               : mousePosition.X < resizeAreaWidth;
    }

    private bool isMouseOverEmptyArea()
    {
        var grid = Mouse.DirectlyOver as Grid;
        if (grid == null)
            return false;

        return grid.Parent == this.controller;
    }


    public void ClosePopup()
    {
        this.controller.Close();
    }

    public void SetViewController(WPF.TimeEntryEditViewController timeEntryEditViewController)
    {
        this.controller = timeEntryEditViewController;

        timeEntryEditViewController.MouseDown += (sender, args) => this.mouseDown(args);
        timeEntryEditViewController.MouseMove += (sender, args) => this.mouseMove(args);
    }

}
}
