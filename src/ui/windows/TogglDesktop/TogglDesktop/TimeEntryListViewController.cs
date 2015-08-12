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

    private string highlightedGUID;

    public int TimerHeight {
        get {
            return this.entriesHost.Top;
        }
    }

    public TimeEntryListViewController()
    {
        InitializeComponent();

        Dock = DockStyle.Fill;

        Toggl.OnTimeEntryList += OnTimeEntryList;
        Toggl.OnLogin += OnLogin;
        Toggl.OnRunningTimerState += te => this.invalidateTimer();
        Toggl.OnStoppedTimerState += this.invalidateTimer;
    }

    private void invalidateTimer()
    {
        this.miniTimerHost.Invalidate();
    }

    public void SetAcceptButton(Form frm)
    {
        // TODO: replace concept of accept buttons (wpf does not have this)
    }

    void OnTimeEntryList(bool open, List<Toggl.TimeEntry> list)
    {
        if (InvokeRequired)
        {
            BeginInvoke((MethodInvoker)delegate
            {
                OnTimeEntryList(open, list);
            });
            return;
        }

        if (!entries.Dispatcher.CheckAccess())
        {
            entries.Dispatcher.BeginInvoke(new Action(() => OnTimeEntryList(open, list)));
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

        this.refreshHighlight();
    }

    void OnLogin(bool open, UInt64 user_id)
    {
        if (InvokeRequired)
        {
            BeginInvoke((MethodInvoker)delegate
            {
                OnLogin(open, user_id);
            });
            return;
        }

        if (!entries.Dispatcher.CheckAccess())
        {
            entries.Dispatcher.BeginInvoke(new Action(() => OnLogin(open, user_id)));
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

    private void refreshHighlight()
    {
        this.HighlightEntry(this.highlightedGUID);
    }

    public void HighlightEntry(string GUID)
    {
        this.highlightedGUID = GUID;
        WPF.TimeEntryCell cell = null;
        if(GUID != null)
            this.cellsByGUID.TryGetValue(GUID, out cell);

        this.entries.HighlightCell(cell);
    }

    public void DisableHighlight()
    {
        this.entries.DisableHighlight();
    }

    public void SetListWidth(int width)
    {
        this.entriesHost.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Top;
        this.entriesHost.Width = width;
    }

    public void DisableListWidth()
    {
        this.entriesHost.Width = this.Width;
        this.entriesHost.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right;
    }

    public void SetManualMode(bool manualMode)
    {
        this.timerEditViewController.SetManualMode(manualMode);
    }
}
}
