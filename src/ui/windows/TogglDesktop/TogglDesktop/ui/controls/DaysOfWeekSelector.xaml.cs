using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace TogglDesktop
{
    public partial class DaysOfWeekSelector : UserControl
    {
        private DayOfWeek lastBeginningOfWeek = DayOfWeek.Monday;
        public DaysOfWeekSelector()
        {
            InitializeComponent();
            GetDaysOfWeekCheckboxes().ForEach(checkBox =>
            {
                checkBox.Checked += OnCheckBoxIsCheckedChanged;
                checkBox.Unchecked += OnCheckBoxIsCheckedChanged;
            });
            RefreshText();
        }

        public static readonly DependencyProperty TextProperty = DependencyProperty.Register(
            "Text", typeof(string), typeof(DaysOfWeekSelector), new PropertyMetadata(default(string)));

        public string Text
        {
            get => (string) GetValue(TextProperty);
            private set => SetValue(TextProperty, value);
        }

        public void SetBeginningOfWeek(DayOfWeek beginningOfWeek)
        {
            if (lastBeginningOfWeek == beginningOfWeek)
            {
                return;
            }

            var daysOfWeekCheckboxes = GetDaysOfWeekCheckboxes();
            var isDayOfWeekChecked = daysOfWeekCheckboxes.Select(c => c.IsChecked).ToArray();

            foreach (var dayOfWeek in DayOfWeekExtensions.DaysOfWeek())
            {
                var newDayPosition = dayOfWeek.PositionRelativeTo(beginningOfWeek);
                var oldDayPosition = dayOfWeek.PositionRelativeTo(lastBeginningOfWeek);
                daysOfWeekCheckboxes[newDayPosition].IsChecked = isDayOfWeekChecked[oldDayPosition];

                daysOfWeekCheckboxes[newDayPosition].Content = Enum.GetName(typeof(DayOfWeek), dayOfWeek);
            }

            lastBeginningOfWeek = beginningOfWeek;
        }

        public void Reset(bool mon, bool tue, bool wed, bool thu, bool fri, bool sat, bool sun)
        {
            lastBeginningOfWeek = DayOfWeek.Monday;
            remindDay1CheckBox.IsChecked = mon;
            remindDay2CheckBox.IsChecked = tue;
            remindDay3CheckBox.IsChecked = wed;
            remindDay4CheckBox.IsChecked = thu;
            remindDay5CheckBox.IsChecked = fri;
            remindDay6CheckBox.IsChecked = sat;
            remindDay7CheckBox.IsChecked = sun;
        }

        public bool[] GetSelection()
        {
            var checkBoxes = GetDaysOfWeekCheckboxes();

            return DayOfWeekExtensions.DaysOfWeek()
                .Select(day => isChecked(checkBoxes[day.PositionRelativeTo(lastBeginningOfWeek)]))
                .ToArray();
        }

        private static bool isChecked(ToggleButton checkBox)
        {
            return checkBox.IsChecked ?? false;
        }

        private void TogglePopup(object sender, RoutedEventArgs e)
        {
            popup.IsOpen = !popup.IsOpen;
        }

        private CheckBox[] GetDaysOfWeekCheckboxes() => new[]
        {
            remindDay1CheckBox,
            remindDay2CheckBox,
            remindDay3CheckBox,
            remindDay4CheckBox,
            remindDay5CheckBox,
            remindDay6CheckBox,
            remindDay7CheckBox,
        };

        private void OnCheckBoxIsCheckedChanged(object sender, RoutedEventArgs e)
        {
            RefreshText();
        }

        private void RefreshText()
        {
            var isDayChecked = GetDaysOfWeekCheckboxes()
                .WithIndex()
                .ToDictionary(x => lastBeginningOfWeek.Add(x.index), x => isChecked(x.item));
            Text = DayOfWeekExtensions.GetText(isDayChecked, lastBeginningOfWeek);
        }
    }
}