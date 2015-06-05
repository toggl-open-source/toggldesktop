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

        Toggl.OnTimeEntryList += OnTimeEntryList;
        Toggl.OnLogin += OnLogin;

        timerEditViewController.DescriptionTextBox.MouseWheel += TimeEntryListViewController_MouseWheel;
        timerEditViewController.DurationTextBox.MouseWheel += TimeEntryListViewController_MouseWheel;

        entries.SetFocusCondition(() => timerEditViewController.CanFocusList());

    }

    void TimeEntryListViewController_MouseWheel(object sender, MouseEventArgs e)
    {
        if (!timerEditViewController.isAutocompleteOpened())
        {
            entriesHost.Focus();
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
        DateTime start = DateTime.Now;

        lock (rendering)
        {
            renderTimeEntryList(list);
        }

        Console.WriteLine(
            "Time entries list view rendered in {0} ms",
            DateTime.Now.Subtract(start).TotalMilliseconds);
    }

    private void renderTimeEntryList(List<Toggl.TimeEntry> list)
    {
        emptyLabel.Visible = (list.Count == 0);

        int maxCount = list.Count;

        for (int i = 0; i < maxCount; i++)
        {
            Toggl.TimeEntry te = list[i];

            WPF.TimeEntryCell cell = null;
            if (entries.Children.Count > i)
            {
                cell = (TogglDesktop.WPF.TimeEntryCell)entries.Children[i];
            }

            if (cell == null)
            {
                cell = new WPF.TimeEntryCell();
                entries.Children.Add(cell);
            }

            cell.Display(te);
        }

        if (entries.Children.Count > list.Count)
        {
            entries.Children.RemoveRange(list.Count, entries.Children.Count - list.Count);
        }

        entriesHost.Invalidate();

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
            entries.Children.Clear();
        }
    }

    private void emptyLabel_Click(object sender, EventArgs e)
    {
        Toggl.OpenInBrowser();
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
