using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace TogglDesktop.AutoCompletion.Items
{
    class TagItem : ModelItem<string>, INotifyPropertyChanged
    {
        private bool _isChecked;
        public bool IsChecked
        {
            get => _isChecked;
            set
            {
                if (_isChecked != value)
                {
                    _isChecked = value;
                    OnPropertyChanged();
                }
            }
        }

        public TagItem(string tag)
            : base(tag, tag, ItemType.TAG)
        {
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}