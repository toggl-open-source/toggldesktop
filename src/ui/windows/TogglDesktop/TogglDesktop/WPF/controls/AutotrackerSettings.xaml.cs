
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    public partial class AutotrackerSettings
    {
        private readonly List<AutotrackerRuleItem> ruleItems = new List<AutotrackerRuleItem>();

        public AutotrackerSettings()
        {
            this.InitializeComponent();

            Toggl.OnAutotrackerRules += this.onAutotrackerRules;
        }

        #region toggl events

        private void onAutotrackerRules(List<Toggl.TogglAutotrackerRuleView> rules, string[] terms)
        {
            if (this.TryBeginInvoke(this.onAutotrackerRules, rules, terms))
                return;

            this.fill(rules, terms);
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
            return;
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

        #endregion

        #region project auto completion

        private void projectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
        }

        private void projectTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
        }

        private void projectDropDownButton_OnClick(object sender, RoutedEventArgs e)
        {
        }

        private void projectAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
        }

        private void projectAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
        }

        #endregion

        #region adding/deleting

        private void onAddButtonClick(object sender, RoutedEventArgs e)
        {
            if (Toggl.AddAutotrackerRule(this.termTextBox.Text, 0))
            {
                this.termTextBox.Clear();
                this.projectTextBox.Clear();
            }
        }

        #endregion

    }
}
