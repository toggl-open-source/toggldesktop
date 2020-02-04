using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using ReactiveUI;

namespace TogglDesktop.AutoCompletion
{
    class CreateProjectButtonItemViewModel : ListBoxItemViewModel
    {
        public ICommand CreateProject { get; }
        public CreateProjectButtonItemViewModel()
        {
            Type = ItemType.CREATE_PROJECT_BUTTON;
            CreateProject = ReactiveCommand.Create<UIElement>(placementTarget =>
            {
                var popupContent = new CreateProjectDialog();
                var popup = new Popup
                {
                    AllowsTransparency = true,
                    Child = popupContent,
                    StaysOpen = false,
                    IsOpen = true,
                    HorizontalOffset = -8,
                    Placement = PlacementMode.Bottom,
                    PlacementTarget = placementTarget
                };
                popupContent.PopupBorder.MinWidth = (placementTarget as FrameworkElement)?.ActualWidth ?? double.NaN;

                var parent = (DependencyObject) placementTarget;
                while (parent != null && !(parent is Panel))
                {
                    parent = VisualTreeHelper.GetParent(parent);
                }

                if (parent is Panel parentPanel)
                {
                    parentPanel.Children.Add(popup);
                    void OnPopupOnClosed(object sender, EventArgs args)
                    {
                        popup.Closed -= OnPopupOnClosed;
                        parentPanel.Children.Remove(popup);
                    }
                    popup.Closed += OnPopupOnClosed;
                }

                popupContent.Focus();
            });
        }
    }
}