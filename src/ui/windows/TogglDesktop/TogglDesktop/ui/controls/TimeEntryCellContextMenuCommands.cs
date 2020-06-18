using System.Windows;
using System.Windows.Input;

namespace TogglDesktop
{
    static class TimeEntryCellContextMenuCommands
    {

        static TimeEntryCellContextMenuCommands ()
        {
            var bindings = new[] {
                new CommandBinding(DeleteCommand, deleteTimeEntry, canDeleteTimeEntry),
                new CommandBinding(UndoDeletionCommand, undoDeletion, canUndoDeletion),
                new CommandBinding(CollapseAllDaysCommand, collapseAllDays, canCollapseAllDays),
                new CommandBinding(ExpandAllDaysCommand, expandAllDays, canExpandAllDays),
            };

            foreach (var binding in bindings)
            {
                CommandManager.RegisterClassCommandBinding(typeof(Window), binding);
            }
        }

        #region delete time entry

        public static readonly RoutedUICommand DeleteCommand =
            new RoutedUICommand("", "DeleteCommand", typeof(TimeEntryCellContextMenuCommands));

        private static void canDeleteTimeEntry(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private static void deleteTimeEntry(object sender, ExecutedRoutedEventArgs e)
        {
            var cell = e.Parameter as TimeEntryCell;
            if (cell == null)
                return;

            cell.ViewModel.DeleteTimeEntry();
        }

        #endregion

        #region undo delete time entry

        public static readonly RoutedUICommand UndoDeletionCommand =
            new RoutedUICommand("", "UndoDeletionCommand", typeof(TimeEntryCellContextMenuCommands));

        private static void canUndoDeletion(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = Toggl.UndoDeletionService.CanUndo;
        }

        private static void undoDeletion(object sender, ExecutedRoutedEventArgs e)
        {
            Toggl.UndoDeletionService.UndoDeletion();
        }

        #endregion

        #region collapse all days

        public static readonly RoutedUICommand CollapseAllDaysCommand =
            new RoutedUICommand("", "CollapseAllDaysCommand", typeof(TimeEntryCellContextMenuCommands));

        private static void canCollapseAllDays(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private static void collapseAllDays(object sender, ExecutedRoutedEventArgs e)
        {
            var window = sender as MainWindow;
            if (window == null)
                return;
            window.GetView<TimerEntryListView>().Entries.CollapseAllDays();
        }

        #endregion

        #region expand all days

        public static readonly RoutedUICommand ExpandAllDaysCommand =
            new RoutedUICommand("", "ExpandAllDaysCommand", typeof(TimeEntryCellContextMenuCommands));

        private static void canExpandAllDays(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private static void expandAllDays(object sender, ExecutedRoutedEventArgs e)
        {
            var window = sender as MainWindow;
            if (window == null)
                return;
            window.GetView<TimerEntryListView>().Entries.ExpandAllDays();
        }



        #endregion
    }
}