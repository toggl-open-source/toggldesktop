
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    public partial class AutotrackerSettings
    {
        private readonly List<AutotrackerRuleItem> ruleItems = new List<AutotrackerRuleItem>();
        private Toggl.TogglAutocompleteView selectedProject;

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
            this.projectAutoComplete.SetController(AutoCompleteControllers.ForProjects(list));
        }

        #endregion

        #region filling from data

        private void fill(List<Toggl.TogglAutotrackerRuleView> rules, string[] terms)
        {
            this.termAutoComplete.SetController(AutoCompleteControllers.ForStrings(terms));

            this.fillRules(rules);
        }

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
                var item = AutotrackerRuleItem.Make(rule.ID, rule.Term, rule.ProjectName);

                this.ruleItems.Add(item);
                this.rulesPanel.Children.Add(item);
            }
        }

        #endregion

        #region term auto completion

        private void termDropDownButton_OnClick(object sender, RoutedEventArgs e)
        {
            var open = this.termDropDownButton.IsChecked ?? false;
            if (open)
            {
                this.termAutoComplete.OpenAndShowAll();
            }
            else
            {
                this.termAutoComplete.IsOpen = false;
                if (!this.termTextBox.IsKeyboardFocused)
                {
                    this.termTextBox.Focus();
                    this.termTextBox.CaretIndex = this.termTextBox.Text.Length;
                }
            }
        }

        private void termAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            this.termDropDownButton.IsChecked = this.termAutoComplete.IsOpen;
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

        private void projectDropDownButton_OnClick(object sender, RoutedEventArgs e)
        {
            var open = this.projectDropDownButton.IsChecked ?? false;
            if (open)
            {
                this.projectAutoComplete.OpenAndShowAll();
            }
            else
            {
                this.projectAutoComplete.IsOpen = false;
                if (!this.projectTextBox.IsKeyboardFocused)
                {
                    this.projectTextBox.Focus();
                    this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
                }
            }
        }

        private void projectAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asProjectItem = e as ProjectItem;
            if (asProjectItem == null)
                return;

            var item = asProjectItem.Item;

            this.selectProject(item);
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            this.selectProject(null);
        }

        private void projectAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            this.projectDropDownButton.IsChecked = this.projectAutoComplete.IsOpen;
        }

        private void selectProject(Toggl.TogglAutocompleteView? item)
        {
            var project = item ?? default(Toggl.TogglAutocompleteView);
            this.selectedProject = project;
            this.projectTextBox.SetText(project.ProjectLabel);
            this.projectColorCircle.Background = new SolidColorBrush(getProjectColor(project.ProjectColor));
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
        }

        #endregion

        #region adding/deleting

        private void onAddButtonClick(object sender, RoutedEventArgs e)
        {
            if (Toggl.AddAutotrackerRule(this.termTextBox.Text, this.selectedProject.ProjectID) != 0)
            {
                this.reset();
            }
        }

        #endregion

        private void reset()
        {
            this.termTextBox.Clear();
            this.projectTextBox.Clear();
            this.selectProject(null);
        }

        private static Color getProjectColor(string colorString)
        {
            var projectColourString = string.IsNullOrEmpty(colorString) ? "#999999" : colorString;
            var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
            return projectColor;
        }

    }
}
