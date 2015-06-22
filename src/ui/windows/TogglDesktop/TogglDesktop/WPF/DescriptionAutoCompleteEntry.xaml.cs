using System;
using System.Windows;
using System.Windows.Media;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    sealed class DescriptionAutoCompleteEntrySample
    {
        public string Description { get { return "Description? Hello. Yes, this is test"; } }

        public Color BackgroundColor { get { return Color.FromRgb(244, 244, 244); } }
    }

    partial class DescriptionAutoCompleteEntry : ISelectable
    {
        private static readonly Color backgroundColorSelected = Color.FromRgb(244, 244, 244);
        private static readonly Color backgroundColor = Color.FromRgb(255, 255, 255);

        private bool selected;

        public DescriptionAutoCompleteEntry(Toggl.AutocompleteItem item, Action selectWithClick)
        {
            this.DataContext = this;
            this.Description = item.Description;
            InitializeComponent();
            this.MouseDown += (sender, args) => selectWithClick();
        }

        #region dependency properties

        public static readonly DependencyProperty DescriptionProperty = DependencyProperty
            .Register("Description", typeof(string), typeof(DescriptionAutoCompleteEntry));

        public string Description
        {
            get { return (string)this.GetValue(DescriptionProperty); }
            set { this.SetValue(DescriptionProperty, value); }
        }
        
        public static readonly DependencyProperty BackgroundColorProperty = DependencyProperty
            .Register("BackgroundColor", typeof(Color), typeof(DescriptionAutoCompleteEntry));


        public Color BackgroundColor
        {
            get { return (Color)this.GetValue(BackgroundColorProperty); }
            set { this.SetValue(BackgroundColorProperty, value); }
        }

        #endregion

        public bool Selected
        {
            get { return this.selected; }
            set
            {
                if (this.selected == value)
                    return;
                this.BackgroundColor = value ? backgroundColorSelected : backgroundColor;
                this.selected = value;
            }
        }

    }
}
