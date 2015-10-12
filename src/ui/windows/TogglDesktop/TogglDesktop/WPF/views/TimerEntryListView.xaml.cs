using System;
using System.Windows;
using System.Collections.Generic;
using TogglDesktop.Diagnostics;

namespace TogglDesktop.WPF
{
    public partial class TimerEntryListView
    {
        private readonly Dictionary<string, TimeEntryCell> cellsByGUID =
            new Dictionary<string, TimeEntryCell>();

        private string highlightedGUID;

        public TimerEntryListView()
        {
            this.InitializeComponent();

            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnTimeEntryList += this.onTimeEntryList;
            Toggl.OnLogin += this.onLogin;
        }

        public double TimerHeight { get { return this.timer.Height; } }

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

            if (open)
            {
                this.entries.Focus(true);
                this.DisableHighlight();
            }
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, te, focusedFieldName))
                return;

            this.highlightEntry(te.GUID);
            if (open)
            {
                this.entries.HighlightKeyboard(te.GUID);
            }
        }


        #endregion

        private void fillTimeEntryList(List<Toggl.TogglTimeEntryView> list)
        {
            var previousCount = this.entries.Children.Count;
            var newCount = list.Count;

            var cells = new List<Tuple<string, TimeEntryCell>>(newCount);

            using (Performance.Measure("rendering time entry list, previous count: {0}, new count: {1}", previousCount, newCount))
            {
                this.cellsByGUID.Clear();

                var children = this.entries.Children;

                // remove superfluous cells
                if (children.Count > list.Count)
                {
                    children.RemoveRange(list.Count, children.Count - list.Count);
                }

                // update existing cells
                var i = 0;
                for (; i < children.Count; i++)
                {
                    var entry = list[i];

                    var cell = (TimeEntryCell)this.entries.Children[i];
                    cell.Display(entry);

                    this.cellsByGUID.Add(entry.GUID, cell);
                    cells.Add(Tuple.Create(entry.GUID, cell));
                }

                // add additional cells
                for (; i < list.Count; i++)
                {
                    var entry = list[i];

                    var cell = new TimeEntryCell();
                    cell.Display(entry);

                    this.cellsByGUID.Add(entry.GUID, cell);
                    cells.Add(Tuple.Create(entry.GUID, cell));

                    children.Add(cell);
                }

                this.entries.FinishedFillingList();
                this.entries.SetTimeEntryCellList(cells);
                this.refreshHighLight();
            }

        }

        private void refreshHighLight()
        {
            this.highlightEntry(this.highlightedGUID);
        }

        private void highlightEntry(string guid)
        {
            this.highlightedGUID = guid;

            TimeEntryCell cell = null;
            if (guid != null)
                this.cellsByGUID.TryGetValue(guid, out cell);

            this.entries.HighlightCell(cell);
        }

        public void SetEditPopup(EditView editView)
        {
            editView.SetTimer(this.timer);
        }

        public void DisableHighlight()
        {
            this.highlightedGUID = null;
            this.entries.DisableHighlight();
        }

        public void SetListWidth(double width)
        {
            this.entries.HorizontalAlignment = HorizontalAlignment.Left;
            this.entries.Width = Math.Max(width, 0);
        }

        public void DisableListWidth()
        {
            this.entries.HorizontalAlignment = HorizontalAlignment.Stretch;
            this.entries.Width = this.Width;
        }

        public void SetManualMode(bool manualMode)
        {
            this.timer.SetManualMode(manualMode);
        }

        private void onFocusTimeEntryList(object sender, EventArgs e)
        {
            this.entries.Focus(true);
        }
        private void onFocusTimer(object sender, EventArgs e)
        {
            this.timer.Focus();
        }
    }
}
