using System;
using System.Collections.Generic;
using System.Windows;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    public partial class TimerEntryListView : IMainView
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

        public double TimerHeight { get { return this.Timer.Height; } }

        protected override void OnInitialized(EventArgs e)
        {
            this.MinHeight = this.Timer.Height;
            this.MinWidth = this.Timer.MinWidth;

            base.OnInitialized(e);
        }

        #region toggl events

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open || userID == 0)
            {
                this.Entries.Children.Clear();
            }
        }

        private void onTimeEntryList(bool open, List<Toggl.TogglTimeEntryView> list, bool showLoadMoreButton)
        {
            if (this.TryBeginInvoke(this.onTimeEntryList, open, list, showLoadMoreButton))
                return;

            this.fillTimeEntryList(list);

            if (open)
            {
                this.Entries.Focus(false);
                this.DisableHighlight();
            }

            this.Entries.SetLoadMoreButtonVisibility(showLoadMoreButton);
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, te, focusedFieldName))
                return;

            using (Performance.Measure("highlighting cell in list"))
            {
                this.highlightEntry(te.GUID);
                if (open)
                {
                    this.Entries.HighlightKeyboard(te.GUID);
                }
            }
        }


        #endregion

        private void fillTimeEntryList(List<Toggl.TogglTimeEntryView> list)
        {
            var previousCount = this.cellsByGUID.Count;
            var newCount = list.Count;

            var cells = new List<Tuple<string, TimeEntryCell>>(newCount);

            using (Performance.Measure("rendering time entry list, previous count: {0}, new count: {1}", previousCount, newCount))
            {
                this.cellsByGUID.Clear();

                Action<string, TimeEntryCell> registerCellByGUID = (guid, cell) =>
                {
                    this.cellsByGUID.Add(guid, cell);
                    cells.Add(Tuple.Create(guid, cell));
                };

                var days = groupByDays(list);

                this.fillDays(days, registerCellByGUID);

                this.Entries.FinishedFillingList();
                this.Entries.SetTimeEntryCellList(cells);
                this.refreshHighLight();
            }

        }

        private void fillDays(List<List<Toggl.TogglTimeEntryView>> days, Action<string, TimeEntryCell> registerCellByGUID)
        {
            var children = this.Entries.Children;

            // remove superfluous days
            if (children.Count > days.Count)
            {
                children.RemoveRange(days.Count, children.Count - days.Count);
            }

            // update existing days
            var i = 0;
            for (; i < children.Count; i++)
            {
                var day = days[i];

                var header = (TimeEntryCellDayHeader)children[i];
                header.Display(day, registerCellByGUID);
            }

            // add additional days
            for (; i < days.Count; i++)
            {
                var day = days[i];

                var header = new TimeEntryCellDayHeader();
                header.Display(day, registerCellByGUID);

                children.Add(header);
            }
        }

        private static List<List<Toggl.TogglTimeEntryView>> groupByDays(List<Toggl.TogglTimeEntryView> list)
        {
            var days = new List<List<Toggl.TogglTimeEntryView>>();
            List<Toggl.TogglTimeEntryView> currentDay = null;

            foreach (var item in list)
            {
                if (item.IsHeader)
                {
                    currentDay = new List<Toggl.TogglTimeEntryView>();
                    days.Add(currentDay);
                }

                currentDay.Add(item);
            }

            return days;
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

            this.Entries.HighlightCell(cell);
        }

        public void DisableHighlight()
        {
            this.highlightedGUID = null;
            this.Entries.DisableHighlight();
        }

        public void SetListWidth(double width)
        {
            this.Entries.HorizontalAlignment = HorizontalAlignment.Left;
            this.Entries.Width = Math.Max(width, 0);
        }

        public void DisableListWidth()
        {
            this.Entries.HorizontalAlignment = HorizontalAlignment.Stretch;
            this.Entries.Width = this.Width;
        }

        public void SetManualMode(bool manualMode)
        {
            this.Timer.SetManualMode(manualMode);
        }

        private void onFocusTimeEntryList(object sender, EventArgs e)
        {
            this.Entries.Focus(true);
        }
        private void onFocusTimer(object sender, EventArgs e)
        {
            this.Timer.Focus();
        }

        public void Activate(bool allowAnimation)
        {
            this.IsEnabled = true;
        }

        public void Deactivate(bool allowAnimation)
        {
            this.IsEnabled = false;
        }
    }
}
