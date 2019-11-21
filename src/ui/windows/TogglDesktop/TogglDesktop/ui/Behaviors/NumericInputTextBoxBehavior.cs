using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interactivity;

namespace TogglDesktop.Behaviors
{
    public class NumericInputTextBoxBehavior : Behavior<TextBox>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.PreviewTextInput += OnPreviewTextInput;
            DataObject.AddPastingHandler(AssociatedObject, OnPaste);
            AssociatedObject.MouseDown += AssociatedObjectOnMouseDown;
        }

        private void AssociatedObjectOnMouseDown(object sender, MouseButtonEventArgs e)
        {
            var point = e.GetPosition(AssociatedObject);
            if (AssociatedObject.ActualWidth - point.X < AssociatedObject.Padding.Right)
            {
                AssociatedObject.CaretIndex = AssociatedObject.Text.Length;
            }
            else if (point.X < AssociatedObject.Padding.Left)
            {
                AssociatedObject.CaretIndex = 0;
            }
        }

        protected override void OnDetaching()
        {
            DataObject.RemovePastingHandler(AssociatedObject, OnPaste);
            AssociatedObject.PreviewTextInput -= OnPreviewTextInput;
            base.OnDetaching();
        }

        private static void OnPaste(object sender, DataObjectPastingEventArgs e)
        {
            if (e.DataObject.GetDataPresent(DataFormats.Text))
            {
                var text = Convert.ToString(e.DataObject.GetData(DataFormats.Text));
                if (!IsValid(text))
                {
                    e.CancelCommand();
                }
            }
            else
            {
                e.CancelCommand();
            }
        }

        private static void OnPreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            e.Handled = !IsValid(e.Text);
        }

        private static bool IsValid(string newText)
        {
            return newText.All(char.IsDigit);
        }
    }
}