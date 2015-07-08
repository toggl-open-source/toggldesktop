using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace TogglDesktop
{
public partial class EditForm : Form
{
    private bool isResizing = false;
    private bool isLeft = false;

    public string GUID = null;
    private WPF.TimeEntryEditViewController controller;
    private bool remainOnTop;

    public EditForm()
    {
        InitializeComponent();
        Padding = new Padding(0, 0, 0, 0);
    }

    const UInt32 HTLEFT = 10;
    const UInt32 HTRIGHT = 11;
    const UInt32 HTBOTTOMRIGHT = 17;
    const UInt32 HTBOTTOM = 15;
    const UInt32 HTBOTTOMLEFT = 16;
    const UInt32 HTTOP = 12;
    const UInt32 HTTOPLEFT = 13;
    const UInt32 HTTOPRIGHT = 14;

    protected override void WndProc(ref Message m)
    {
        const UInt32 WM_NCHITTEST = 0x0084;
        const UInt32 WM_MOUSEMOVE = 0x0200;


        const int RESIZE_HANDLE_SIZE = 10;

        bool handled = false;

        if (m.Msg == WM_NCHITTEST || m.Msg == WM_MOUSEMOVE)
        {
            Size formSize = Size;
            Point screenPoint = new Point(m.LParam.ToInt32());
            Point clientPoint = PointToClient(screenPoint);

            Dictionary<UInt32, Rectangle> boxes = new Dictionary<UInt32, Rectangle>() {
                {HTBOTTOMLEFT, new Rectangle(0, formSize.Height - RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE)},
                {HTBOTTOM, new Rectangle(RESIZE_HANDLE_SIZE, formSize.Height - RESIZE_HANDLE_SIZE, formSize.Width - 2*RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE)},
                {HTBOTTOMRIGHT, new Rectangle(formSize.Width - RESIZE_HANDLE_SIZE, formSize.Height - RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE)},
                {HTRIGHT, new Rectangle(formSize.Width - RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE, formSize.Height - 2*RESIZE_HANDLE_SIZE)},
                {HTTOPRIGHT, new Rectangle(formSize.Width - RESIZE_HANDLE_SIZE, 0, RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE) },
                {HTTOP, new Rectangle(RESIZE_HANDLE_SIZE, 0, formSize.Width - 2*RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE) },
                {HTTOPLEFT, new Rectangle(0, 0, RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE) },
                {HTLEFT, new Rectangle(0, RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE, formSize.Height - 2*RESIZE_HANDLE_SIZE) }
            };

            foreach (KeyValuePair<UInt32, Rectangle> hitBox in boxes)
            {
                if (hitBox.Value.Contains(clientPoint))
                {
                    if (isLeft)
                    {
                        if (hitBox.Key == HTBOTTOMLEFT || hitBox.Key == HTTOPLEFT || hitBox.Key == HTLEFT)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        if (hitBox.Key == HTBOTTOMRIGHT || hitBox.Key == HTTOPRIGHT || hitBox.Key == HTRIGHT)
                        {
                            continue;
                        }
                    }
                    m.Result = (IntPtr)hitBox.Key;
                    handled = true;
                    break;
                }
            }
        }

        if (!handled)
        {
            base.WndProc(ref m);
        }
    }

    internal void setPlacement(bool left, Point p, int height)
    {
        this.controller.SetShadow(left, height);

        height = Math.Max(400, height);

        TopMost = true;
        isLeft = !left;
        Height = height;


        if (left)
        {
            p.X -= Width;
            resizeHandle.Cursor = Cursors.SizeNS;
        }
        else
        {
            resizeHandle.Cursor = Cursors.SizeNWSE;
        }
        Location = p;
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
        this.SetWindowPos(remainOnTop);
    }

    public void SetWindowPos(bool onTop)
    {
        this.remainOnTop = onTop;
        Win32.SetWindowPos(Handle, onTop ? Win32.HWND_TOPMOST : Win32.HWND_NOTOPMOST, 0, 0, 0, 0, Win32.SWP_NOMOVE | Win32.SWP_NOSIZE);
    }

    private void resizeHandle_MouseDown(object sender, MouseEventArgs e)
    {
        isResizing = true;
    }

    private void resizeHandle_MouseMove(object sender, MouseEventArgs e)
    {
        if (!isResizing)
        {
            return;
        }
        isResizing = (e.Button == MouseButtons.Left);
        Win32.ReleaseCapture();
        int location = (int)((!isLeft) ? HTBOTTOM : HTBOTTOMRIGHT);
        if (isResizing)
        {
            Win32.SendMessage(Handle, Win32.wmNcLButtonDown, location, 0);
        }
        else
        {
            Win32.SendMessage(Handle, Win32.wmNcLButtonUp, location, 0);
        }
    }

    public void ClosePopup()
    {
        // TODO: what was this doing before?
    }

    public void SetViewController(WPF.TimeEntryEditViewController timeEntryEditViewController)
    {
        this.controller = timeEntryEditViewController;
    }
}
}
