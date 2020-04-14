using System;
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
            Text = "every day";
            GetDaysOfWeekCheckboxes().ForEach(checkBox =>
            {
                checkBox.Checked += OnCheckBoxIsCheckedChanged;
                checkBox.Unchecked += OnCheckBoxIsCheckedChanged;
            });
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

        private static string GetText(bool[] daysOfWeek)
        {
            // TODO: use beginning of week for correct order of days
            var checkedCount = daysOfWeek.Count(x => x);
            return daysOfWeek switch
            {
                _ when checkedCount == 7
                    => "every day",
                var x when checkedCount == 5 && !x[0] && !x[6]
                    => "on weekdays",
                var x when checkedCount == 6 && (!x[0] || !x[6])
                    => "on weekdays and " + (x[0] ? "Sunday" : "Saturday"),
                _ when checkedCount >= 5
                    => "every day except " + string.Join(", ",
                    daysOfWeek.WithIndex()
                        .Where(x => !x.item)
                        .Select(x => Enum.GetName(typeof(DayOfWeek), (DayOfWeek)x.index))),
                var x when checkedCount == 4 && !x[0] && !x[6]
                    => "on weekdays except " + Enum.GetName(typeof(DayOfWeek), (DayOfWeek)(daysOfWeek.WithIndex().Skip(1).First(d => !d.item).index)),
                var x when checkedCount == 2 && x[0] && x[6]
                    => "on weekend",
                _ => "on " + string.Join(", ",
                    daysOfWeek.WithIndex()
                    .Where(x => x.item)
                    .Select(x => Enum.GetName(typeof(DayOfWeek), (DayOfWeek)x.index)))
            };
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
            var isCheckBoxChecked = GetDaysOfWeekCheckboxes().Select(isChecked).ToArray();
            Text = GetText(isCheckBoxChecked.Skip(6).Concat(isCheckBoxChecked.Take(6)).ToArray());
        }
    }
}