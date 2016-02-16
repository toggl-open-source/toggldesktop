using System;
using System.Windows.Controls;

namespace TogglDesktop.Experiments
{
    static class ExperimentHacks
    {
        public static void RemoveEmptyStateFirstLine(IExperiment experiment, ExperimentParameters parameters)
        {
            if (parameters.TutorialManager.MainWindow.TryBeginInvoke(RemoveEmptyStateFirstLine, experiment, parameters))
                return;

            var emptyStateButton = parameters.TutorialManager
                .MainWindow
                .GetView<TimerEntryListView>()
                .Entries
                .FindName("emptyListText") as Button;

            if (emptyStateButton != null)
            {
                var stackPanel = emptyStateButton.Content as StackPanel;

                if (stackPanel != null)
                {
                    if (stackPanel.Children.Count == 3)
                    {
                        stackPanel.Children[0].RemoveFromParent();
                    }
                    return;
                }
            }
            throw new Exception(string.Format("Unable to modify empty state in experiment {0}.", experiment.Id));
        } 
    }
}