
using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Windows.Threading;
using TogglDesktop.Diagnostics;
using HorizontalAlignment = System.Windows.HorizontalAlignment;

namespace TogglDesktop.WPF
{
    public partial class TimerEntryListView
    {
        private readonly Dictionary<string, TimeEntryCell> cellsByGUID =
            new Dictionary<string, TimeEntryCell>();

        public TimerEntryListView()
        {
            this.InitializeComponent();

            Toggl.OnTimeEntryList += this.onTimeEntryList;
            Toggl.OnLogin += this.onLogin;
        }

        protected override void OnInitialized(EventArgs e)
        {
            this.MinHeight = this.timer.Height;
            this.MinWidth = this.timer.MinWidth;

            base.OnInitialized(e);
        }

        #region toggl events

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open || userID == 0)
            {
                this.entries.Children.Clear();
            }
        }

        private void onTimeEntryList(bool open, List<Toggl.TogglTimeEntryView> list)
        {
            if (this.TryBeginInvoke(this.onTimeEntryList, open, list))
                return;

            this.fillTimeEntryList(list);
        }


        #endregion

        private void fillTimeEntryList(List<Toggl.TogglTimeEntryView> list)
        {
            var previousCount = this.entries.Children.Count;
            var newCount = list.Count;

            using (Performance.Measure("rendering time entry list, previous count: {0}, new count: {1}", previousCount, newCount))
            {
                this.cellsByGUID.Clear();

                int maxCount = list.Count;

                for (int i = 0; i < maxCount; i++)
                {
                    var entry = list[i];

                    TimeEntryCell cell = null;
                    if (this.entries.Children.Count > i)
                    {
                        cell = (TimeEntryCell)this.entries.Children[i];
                    }

                    if (cell == null)
                    {
                        cell = new WPF.TimeEntryCell();
                        this.entries.Children.Add(cell);
                    }
                    cell.Display(entry);
                    this.cellsByGUID.Add(entry.GUID, cell);
                }

                if (this.entries.Children.Count > list.Count)
                {
                    this.entries.Children.RemoveRange(list.Count, this.entries.Children.Count - list.Count);
                }

                this.entries.FinishedFillingList();

                this.refreshHighlight();
            }

        }

        private void refreshHighlight()
        {
            this.entries.RefreshHighLight();
        }

        public void SetEditPopup(EditView editView)
        {
            editView.SetTimer(this.timer);
        }

        public void HighlightEntry(string guid)
        {
            TimeEntryCell cell = null;
            if (guid != null)
                this.cellsByGUID.TryGetValue(guid, out cell);

            this.entries.HighlightCell(cell);
        }

        public void DisableHighlight()
        {
            this.entries.DisableHighlight();
        }

        public void SetListWidth(int width)
        {
            this.entries.HorizontalAlignment = HorizontalAlignment.Left;
            this.entries.Width = width;
        }

        public void DisableListWidth()
        {
            this.entries.HorizontalAlignment = HorizontalAlignment.Stretch;
        }

        public void SetManualMode(bool manualMode)
        {
            this.timer.SetManualMode(manualMode);
        }
    }
}
