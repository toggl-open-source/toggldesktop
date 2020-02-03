namespace TogglDesktop.AutoCompletion.Implementation
{
    class StringItem : SimpleItem<string>
    {
        public StringItem(string item)
            : base(item, item)
        {
        }
    }
}