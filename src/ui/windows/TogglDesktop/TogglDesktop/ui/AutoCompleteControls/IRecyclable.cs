using System.Collections.Generic;

namespace TogglDesktop.AutoCompleteControls
{
    interface IRecyclable
    {
        void Recycle();
    }

    static class RecyclableExtensions
    {
        public static T MarkForRecycling<T>(this T recyclable, List<IRecyclable> recyclables)
            where T : IRecyclable
        {
            recyclables.Add(recyclable);
            return recyclable;
        }
    }
}