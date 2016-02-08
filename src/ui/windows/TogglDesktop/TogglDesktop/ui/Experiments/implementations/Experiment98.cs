using System;
using System.CodeDom;
using System.Windows.Controls;

namespace TogglDesktop.Experiments
{
    sealed class Experiment98 : ExperimentBase
    {
        public Experiment98()
            : base(98, false)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters) 
        {
            if (parameters.IsFirstRun)
            {
                parameters.TutorialManager.ActivateScreen<Experiment98Screen1>();
                this.pruneEmptyState(parameters);
            }

            Toggl.OnUserTimeEntryStart += this.sendStartAction;
        }

        private void pruneEmptyState(ExperimentParameters parameters)
        {
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
                    stackPanel.Children[0].RemoveFromParent();
                    return;
                }
            }
            throw new Exception("Unable to modify empty state in experiment 98.");
        }

        private void sendStartAction()
        {
            Toggl.SendObmAction(this.Id, "start");

            Toggl.OnUserTimeEntryStart -= this.sendStartAction;
        }
    }
}