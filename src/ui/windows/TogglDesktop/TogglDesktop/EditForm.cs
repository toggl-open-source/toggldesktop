using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class EditForm : Form
    {
        private const int SWP_NOMOVE = 0x0002;
        private const int SWP_NOSIZE = 0x0001;

        private const int wmNcLButtonDown = 0xA1;
        private const int wmNcLButtonUp = 0xA2;
        private const int HtBottomRight = 17;
        private const int HtBottom = 15;

        private bool isResizing = false;

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetWindowPos(IntPtr hWnd,
            int hWndInsertAfter, int x, int u, int cx, int cy, int uFlags);

        [DllImport("user32.dll")]
        private static extern int ReleaseCapture();

        [DllImport("user32.dll")]
        private static extern int SendMessage(IntPtr hwnd, int msg, int wparam, int lparam);

        public EditForm()
        {
            InitializeComponent();
            Padding = new System.Windows.Forms.Padding(11, 0, 11, 4);
            labelArrowLeft.Width = labelArrowRight.Width = 13;
            labelArrowLeft.Height = labelArrowRight.Height = 24;
            labelArrowRight.Location = new Point(Width-13, labelArrowRight.Location.Y);
            CancelButton = CloseButton;
        }
        public string GUID = null;
        public TimeEntryEditViewController editView;

        public void CloseButton_Click(object sender, EventArgs e)
        {
            if (editView.dropDownsClosed())
            {
                ClosePopup();
            }
        }

        protected override void WndProc(ref Message m)
        {
            const UInt32 WM_NCHITTEST = 0x0084;
            const UInt32 WM_MOUSEMOVE = 0x0200;

            const UInt32 HTLEFT = 10;
            const UInt32 HTRIGHT = 11;
            const UInt32 HTBOTTOMRIGHT = 17;
            const UInt32 HTBOTTOM = 15;
            const UInt32 HTBOTTOMLEFT = 16;
            const UInt32 HTTOP = 12;
            const UInt32 HTTOPLEFT = 13;
            const UInt32 HTTOPRIGHT = 14;

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
                        if (labelArrowLeft.Visible)
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

        internal void setPlacement(bool left, int arrowTop, Point p, Screen s, MainWindowController main)
        {
            TopMost = true;
            labelArrowLeft.Visible = !left;
            labelArrowRight.Visible = left;

            if (left)
            {
                resizeHandle.Cursor = System.Windows.Forms.Cursors.SizeNS;
            }
            else
            {
                resizeHandle.Cursor = System.Windows.Forms.Cursors.SizeNWSE;
            }

            int posY = ((arrowTop != 0) ? arrowTop: (Height / 2)) - (labelArrowRight.Height / 2);

            if (p.Y < s.WorkingArea.Location.Y)
            {
                posY -= Math.Abs(s.WorkingArea.Location.Y - p.Y) - 10;
                p.Y = s.WorkingArea.Location.Y + 10;
            }
            if (p.Y + Height >= s.WorkingArea.Height)
            {
                int newPosY = s.WorkingArea.Height - Height;
                posY += Math.Abs(p.Y) - newPosY;
                p.Y = newPosY;
            }
            if ((posY > Height - labelArrowLeft.Height - 5) ||
                (p.Y + posY) < main.Location.Y ||
                main.Location.Y + main.Height < p.Y + posY
                )
            {
                ClosePopup();
            }

            labelArrowRight.Location = new Point(labelArrowRight.Location.X, posY);
            labelArrowLeft.Location = new Point(labelArrowLeft.Location.X, posY);           

            Location = p;
        }

        internal void reset()
        {
            editView.resetForms();
        }

        internal void setWindowPos(int HWND_TOPMOST)
        {
            SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        internal void ClosePopup()
        {
            editView.buttonDone_Click(null, null);
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
            ReleaseCapture();
            int location = (labelArrowRight.Visible) ? HtBottom : HtBottomRight;
            if (isResizing)
            {
                SendMessage(Handle, wmNcLButtonDown, location, 0);
            }
            else
            {
                SendMessage(Handle, wmNcLButtonUp, location, 0);
            }
        }
    }
}
