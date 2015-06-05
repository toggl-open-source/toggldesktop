using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace TogglDesktop
{
public partial class TimeEntryListViewController : UserControl
{
    private Object rendering = new Object();

    public TimeEntryListViewController()
    {
        InitializeComponent();

        Dock = DockStyle.Fill;

        //entriesHost.AutoScroll = false;
        //entriesHost.HorizontalScroll.Enabled = false;
        //entriesHost.AutoScroll = true;

        Toggl.OnTimeEntryList += OnTimeEntryList;
        Toggl.OnLogin += OnLogin;

        timerEditViewController.DescriptionTextBox.MouseWheel += TimeEntryListViewController_MouseWheel;
        timerEditViewController.DurationTextBox.MouseWheel += TimeEntryListViewController_MouseWheel;
    }

    void TimeEntryListViewController_MouseWheel(object sender, MouseEventArgs e)
    {
        if (!timerEditViewController.isAutocompleteOpened())
        {
            entriesHost.Focus();
        }
    }

    public int EntriesTop
    {
        get {
            return entriesHost.Location.Y;
        }
    }

    public void SetAcceptButton(Form frm)
    {
        timerEditViewController.SetAcceptButton(frm);
    }

    void OnTimeEntryList(bool open, List<Toggl.TimeEntry> list)
    {
        if (InvokeRequired)
        {
            Invoke((MethodInvoker)delegate {
                OnTimeEntryList(open, list);
            });
            return;
        }
        //if (open && this.CurrentEntry != null)
        //{
        //    this.CurrentEntry.opened = false;
        //}
        DateTime start = DateTime.Now;

        lock (rendering)
        {
            renderTimeEntryList(list);
        }

        Console.WriteLine(String.Format(
            "Time entries list view rendered in {0} ms",
            DateTime.Now.Subtract(start).TotalMilliseconds));
    }

    private void renderTimeEntryList(List<Toggl.TimeEntry> list)
    {
        emptyLabel.Visible = (list.Count == 0);
        entriesHost.SuspendLayout();

        // Hide entry list for initial loading to avoid crazy flicker
        //if (entries.Controls.Count == 0)
        //{
        //    entries.Visible = false;
        //}

        // We cannot render more than N time entries using winforms,
        // because we run out of window handles. As a temporary fix,
        // dont even attempt to render more than N time entries.
        int maxCount = Math.Min(200, list.Count);

        int k = 0;
        for (int j = 0; j < 1; j++)
        {
            for (int i = 0; i < maxCount; i++)
            {
                Toggl.TimeEntry te = list.ElementAt(i);

                WPF.TimeEntryCell cell = null;
                if (entries.Children.Count > i)
                {
                    //cell = entries.Children[i] as TogglDesktop.WPF.TimeEntryCell;
                }

                if (cell == null)
                {
                    cell = new WPF.TimeEntryCell(this);
                    entries.Children.Add(cell);
                    //cell.Width = entriesHost.Width;
                }

                cell.Display(te);
                //entries.Controls.SetChildIndex(cell, i);
                k++;
            }
        }

        Console.WriteLine("entries: " + k);

        //while (entries.Children.Count > list.Count)
        //{
        //    entries.Children[list.Count].Dispose();
        //    // Dispose() will remove the control from collection
        //}

        entriesHost.ResumeLayout();
        entriesHost.PerformLayout();

        if (!entriesHost.Visible)
        {
            entriesHost.Visible = true;
        }
    }

    private void TimeEntryListViewController_Load(object sender, EventArgs e)
    {
    }

    void OnLogin(bool open, UInt64 user_id)
    {
        if (InvokeRequired)
        {
            Invoke((MethodInvoker)delegate {
                OnLogin(open, user_id);
            });
            return;
        }
        if (open || user_id == 0)
        {
            entriesHost.SuspendLayout();
            entriesHost.Controls.Clear();
            entriesHost.ResumeLayout();
            entriesHost.PerformLayout();
        }
    }

    private void entries_ClientSizeChanged(object sender, EventArgs e)
    {
        if (entriesHost.Controls.Count > 0)
        {
            entriesHost.SuspendLayout();
            entriesHost.Controls[0].Width = entriesHost.ClientSize.Width;
            entriesHost.ResumeLayout();
        }
    }

    private void entries_MouseEnter(object sender, EventArgs e)
    {
        if (!timerEditViewController.focusList()) {
            entriesHost.Focus();
        }
    }

    private void emptyLabel_Click(object sender, EventArgs e)
    {
        Toggl.OpenInBrowser();
    }

    internal FlowLayoutPanel getListing()
    {
        return null;
        //return entriesHost;
    }

    internal WPF.TimeEntryCell findControlByGUID(string GUID)
    {
        return this.entries.Children
            .Cast<WPF.TimeEntryCell>()
            .FirstOrDefault(child => child.GUID == GUID);
        if (timerEditViewController.durationFocused)
        {
            return this.entries.Children
                .Cast<WPF.TimeEntryCell>()
                .FirstOrDefault(child => child.GUID == GUID);
        }
        return null;
    }

    internal void setEditPopup(EditForm editForm)
    {
        timerEditViewController.editForm = editForm;
    }
}
}
