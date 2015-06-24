
using System.Collections.Generic;

namespace TogglDesktop
{
    static class LinqExtensions
    {
        public static IEnumerable<T> Yield<T>(this T subject)
        {
            yield return subject;
        }

        public static IEnumerable<T> Prepend<T>(this T subject, IEnumerable<T> sequence)
        {
            yield return subject;

            foreach (var obj in sequence)
            {
                yield return obj;
            }
        }

    }
}
