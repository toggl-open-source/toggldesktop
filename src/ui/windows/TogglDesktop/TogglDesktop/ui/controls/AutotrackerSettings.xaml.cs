using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop
{
    public partial class AutotrackerSettings
    {
        private readonly List<AutotrackerRuleItem> _ruleItems = new List<AutotrackerRuleItem>();
        private Toggl.TogglAutocompleteView _selectedProject;
        private int _selectedRuleId = -1;

        public AutotrackerSettings()
        {
            this.InitializeComponent();

            Toggl.OnAutotrackerRules += this.onAutotrackerRules;
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;
        }

        #region toggl events

        private void onAutotrackerRules(List<Toggl.TogglAutotrackerRuleView> rules, string[] terms)
        {
            if (this.TryBeginInvoke(this.onAutotrackerRules, rules, terms))
                return;

            this.fill(rules, terms);
        }

        private void onProjectAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onProjectAutocomplete, list))
                return;

            this.projectAutoComplete.SetController(AutoCompleteControllersFactory.ForProjects(list));
        }

        #endregion

        #region filling from data

        private void fill(List<Toggl.TogglAutotrackerRuleView> rules, string[] terms)
        {
            // this.termAutoComplete.SetController(AutoCompleteControllers.ForStrings(terms));
            this.fillRules(rules);
        }

        private void fillRules(List<Toggl.TogglAutotrackerRuleView> rules)
        {
            this.rulesPanel.Children.Clear();

            foreach (var item in this._ruleItems)
            {
                item.Recycle();
            }

            this._ruleItems.Clear();

            foreach (var rule in rules)
            {
                var item = AutotrackerRuleItem.Make(rule.ID, rule.Term, rule.ProjectAndTaskLabel);

                this._ruleItems.Add(item);
                this.rulesPanel.Children.Add(item);
            }

            if (this._selectedRuleId != -1)
            {
                var id = Math.Min(this._selectedRuleId, this._ruleItems.Count - 1);
                this._selectedRuleId = -1;
                this.selectRule(id);
            }
        }

        #endregion

        #region project auto completion

        private void projectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!string.IsNullOrEmpty(this._selectedProject.ProjectLabel))
            {
                this.selectProject(this._selectedProject);
            }
        }

        private void projectAutoComplete_OnConfirmCompletion(object sender, IAutoCompleteItem e)
        {
            if (e is IModelItem<Toggl.TogglAutocompleteView> modelItemViewModel)
            {
                this.selectProject(modelItemViewModel.Model);
            }
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            this.selectProject(null);
        }

        private void selectProject(Toggl.TogglAutocompleteView? item)
        {
            var project = item ?? default;
            this._selectedProject = project;
            this.projectTextBox.SetText(project.ProjectLabel, project.TaskLabel);
            this.projectColorCircle.Background = Utils.AdaptedProjectColorBrushFromString(project.ProjectColor);
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
        }

        #endregion

        #region adding/deleting

        private void onAnyTextBoxKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                e.Handled = true;
                this.tryAdd();
            }
        }

        private void onAddButtonClick(object sender, RoutedEventArgs e)
        {
            this.tryAdd();
        }

        private void tryAdd()
        {
            if (string.IsNullOrWhiteSpace(this.termTextBox.Text))
            {
                this.termTextBox.Focus();
                return;
            }

            if (this._selectedProject.ProjectID == 0 && this._selectedProject.TaskID == 0)
            {
                this.projectTextBox.Focus();
                return;
            }

            if (Toggl.AddAutotrackerRule(
                this.termTextBox.Text,
                this._selectedProject.ProjectID,
                this._selectedProject.TaskID) != 0)
            {
                this.reset();
            }
        }

        #endregion

        #region keyboard list navigation

        private void onListGotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this._selectedRuleId == -1 && this._ruleItems.Count > 0)
            {
                this.selectRule(0);
            }
        }

        private void onListLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.selectRule(-1);
        }

        private void onListPreviewKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Up:
                case Key.Tab when Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift):
                    {
                        if (this._selectedRuleId == 0)
                            break;
                        if (this._selectedRuleId == -1)
                            this.selectRule(0);
                        else if (this._selectedRuleId > 0)
                            this.selectRule(this._selectedRuleId - 1);
                        e.Handled = true;
                        break;
                    }

                case Key.Down:
                case Key.Tab when !(Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift)):
                    {
                        if (this._selectedRuleId == this._ruleItems.Count - 1)
                            break;
                        if (this._selectedRuleId == -1)
                            this.selectRule(0);
                        else if (this._selectedRuleId < this._ruleItems.Count - 1)
                            this.selectRule(this._selectedRuleId + 1);
                        e.Handled = true;
                        break;
                    }

                case Key.Delete:
                case Key.Back:
                    {
                        if (this._selectedRuleId != -1)
                            this.selectedRule.DeleteRule();
                        e.Handled = true;
                        break;
                    }
            }
        }

        private AutotrackerRuleItem selectedRule
        {
            get { return this._ruleItems[this._selectedRuleId]; }
        }

        private void selectRule(int i)
        {
            if (this._selectedRuleId != -1)
                this.selectedRule.IsSelected = false;

            this._selectedRuleId = i;

            if (this._selectedRuleId != -1)
                this.selectedRule.IsSelected = true;
        }

        #endregion

        private void reset()
        {
            this.termTextBox.Clear();
            this.projectTextBox.Clear();
            this.selectProject(null);
        }
    }
}
