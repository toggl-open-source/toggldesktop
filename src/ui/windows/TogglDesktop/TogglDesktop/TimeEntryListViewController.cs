using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows.Forms;
using System.Windows.Threading;
using TogglDesktop.WPF;

namespace TogglDesktop
{
public partial class TimeEntryListViewController : UserControl
{
    private Object rendering = new Object();

    private readonly Dictionary<string, WPF.TimeEntryCell> cellsByGUID =
        new Dictionary<string, TimeEntryCell>();

    public TimeEntryListViewController()
    {
        InitializeComponent();

        Dock = DockStyle.Fill;

        Toggl.OnTimeEntryList += OnTimeEntryList;
        Toggl.OnLogin += OnLogin;

        //timerEditViewController.DescriptionTextBox.MouseWheel += TimeEntryListViewController_MouseWheel;
        //timerEditViewController.DurationTextBox.MouseWheel += TimeEntryListViewController_MouseWheel;

        entries.SetFocusCondition(() => timerEditViewController.CanFocusList());
    }

    void TimeEntryListViewController_MouseWheel(object sender, MouseEventArgs e)
    {
        //if (!timerEditViewController.isAutocompleteOpened())
        //{
        //    entriesHost.Focus();
        //}
    }

    public void SetAcceptButton(Form frm)
    {
        // TODO: replace concept of accept buttons (wpf does not have this)
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

        this.cellsByGUID.Clear();

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
            this.cellsByGUID.Add(te.GUID, cell);
        }

        if (entries.Children.Count > list.Count)
        {
            entries.Children.RemoveRange(list.Count, entries.Children.Count - list.Count);
        }

        entries.Dispatcher.Invoke(() => { }, DispatcherPriority.Render);
        entriesHost.Invalidate();

        entries.RefreshHighLight();
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

    public void SetEditPopup(WPF.TimeEntryEditViewController editView)
    {
        editView.SetTimer(this.timerEditViewController);
    }

    public void HighlightEntry(string GUID)
    {
        WPF.TimeEntryCell cell = null;
        if(GUID != null)
            this.cellsByGUID.TryGetValue(GUID, out cell);

        this.entries.HighlightCell(cell);
    }

    public void DisableHighlight()
    {
        this.entries.DisableHighlight();
    }

}
}
