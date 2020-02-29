using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class TimeEntryLabel : UserControl
    {
        public TimeEntryLabelViewModel ViewModel
        {
            get => (TimeEntryLabelViewModel)DataContext;
            set => DataContext = value;
        }

        public TimeEntryLabel()
        {
            InitializeComponent();
        }

        public event MouseButtonEventHandler DescriptionLabelMouseDown;
        public event MouseButtonEventHandler ProjectLabelMouseDown;

        private double _verticalPadding;
        public double VerticalPadding
        {
            get => _verticalPadding;
            set
            {
                _verticalPadding = value;
                descriptionLabel.Margin = new Thickness(0, 0, 0, _verticalPadding);
                addDescriptionLabel.Margin = new Thickness(0, 0, 0, _verticalPadding);
                projectLabel.Margin = new Thickness(0, _verticalPadding, 0, 0);
            }
        }

        private bool _compactDescription;
        public bool CompactDescription
        {
            get => _compactDescription;
            set
            {
                _compactDescription = value;
                if (_compactDescription)
                {
                    descriptionLabel.LineHeight = 16;
                    descriptionLabel.LineStackingStrategy = LineStackingStrategy.BlockLineHeight;
                    addDescriptionLabel.LineHeight = 16;
                    addDescriptionLabel.LineStackingStrategy = LineStackingStrategy.BlockLineHeight;
                }
            }
        }

        private void onDescriptionLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            DescriptionLabelMouseDown?.Invoke(sender, e);
        }

        private void onProjectLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            ProjectLabelMouseDown?.Invoke(sender, e);
        }
    }
}