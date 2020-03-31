﻿using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop
{
    public partial class AutotrackerSettings
    {
        private readonly List<AutotrackerRuleItem> ruleItems = new List<AutotrackerRuleItem>();
        private Toggl.TogglAutocompleteView selectedProject;
        private int selectedRuleId = -1;

        public AutotrackerSettings()
        {
            this.InitializeComponent();

            Toggl.OnAutotrackerRules.ObserveOnDispatcher().Subscribe(this.fillRules);
            Toggl.OnProjectAutocomplete
                .Select(AutoCompleteControllersFactory.ForProjects)
                .ObserveOnDispatcher()
                .Subscribe(this.projectAutoComplete.SetController);
        }

        #region filling from data

        private void fillRules(List<Toggl.TogglAutotrackerRuleView> rules)
        {
            this.rulesPanel.Children.Clear();

            foreach (var item in this.ruleItems)
            {
                item.Recycle();
            }
            this.ruleItems.Clear();

            foreach (var rule in rules)
            {
                var item = AutotrackerRuleItem.Make(rule.ID, rule.Term, rule.ProjectAndTaskLabel);

                this.ruleItems.Add(item);
                this.rulesPanel.Children.Add(item);
            }

            if (this.selectedRuleId != -1)
            {
                var id = Math.Min(this.selectedRuleId, this.ruleItems.Count - 1);
                this.selectedRuleId = -1;
                this.selectRule(id);
            }
        }

        #endregion

        #region project auto completion

        private void projectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!string.IsNullOrEmpty(this.selectedProject.ProjectLabel))
            {
                this.selectProject(this.selectedProject);
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
            var project = item ?? default(Toggl.TogglAutocompleteView);
            this.selectedProject = project;
            this.projectTextBox.SetText(project.ProjectLabel, project.TaskLabel);
            this.projectColorCircle.Background = Utils.ProjectColorBrushFromString(project.ProjectColor);
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
            if (this.selectedProject.ProjectID == 0 && this.selectedProject.TaskID == 0)
            {
                this.projectTextBox.Focus();
                return;
            }

            if (Toggl.AddAutotrackerRule(
                this.termTextBox.Text,
                this.selectedProject.ProjectID,
                this.selectedProject.TaskID) != 0)
            {
                this.reset();
            }
        }

        #endregion

        #region keyboard list navigation

        private void onListGotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.selectedRuleId == -1 && this.ruleItems.Count > 0)
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
                    if (this.selectedRuleId == 0)
                        break;
                    if (this.selectedRuleId == -1)
                        this.selectRule(0);
                    else if(this.selectedRuleId > 0)
                        this.selectRule(this.selectedRuleId - 1);
                    e.Handled = true;
                    break;
                }
                case Key.Down:
                case Key.Tab when !(Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift)):
                {
                    if (this.selectedRuleId == this.ruleItems.Count - 1)
                        break;
                    if (this.selectedRuleId == -1)
                        this.selectRule(0);
                    else if (this.selectedRuleId < this.ruleItems.Count - 1)
                        this.selectRule(this.selectedRuleId + 1);
                    e.Handled = true;
                    break;
                }
                case Key.Delete:
                case Key.Back:
                {
                    if(this.selectedRuleId != -1)
                        this.selectedRule.DeleteRule();
                    e.Handled = true;
                    break;
                }
            }
        }

        private AutotrackerRuleItem selectedRule
        {
            get { return this.ruleItems[this.selectedRuleId]; }
        }

        private void selectRule(int i)
        {
            if (this.selectedRuleId != -1)
                this.selectedRule.IsSelected = false;

            this.selectedRuleId = i;
            
            if (this.selectedRuleId != -1)
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
