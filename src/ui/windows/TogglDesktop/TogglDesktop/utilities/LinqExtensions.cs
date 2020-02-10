
using System;
using System.Collections.Generic;
using System.Linq;

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

    public static List<T> GetCount<T>
    (this List<T> list, out int count)
    {
        count = list.Count;
        return list;
    }

    // from https://github.com/beardgame/utilities/blob/develop/src/Linq/Extensions.cs
    public static T RandomElement<T>(this IEnumerable<T> source, Random random)
    {
        if (source == null)
            throw new ArgumentNullException("source");

        // optimisation for collections
        var asCollection = source as ICollection<T>;
        if (asCollection != null)
        {
            if (asCollection.Count == 0)
                throw new InvalidOperationException("Sequence was empty.");
            return asCollection.ElementAt(random.Next(asCollection.Count));
        }

        var current = default(T);
        var count = 0;
        foreach (var element in source)
        {
            count++;
            if (random.Next(count) == 0)
            {
                current = element;
            }
        }
        if (count == 0)
        {
            throw new InvalidOperationException("Sequence was empty.");
        }
        return current;
    }

    public static void ForEach<T>(this IEnumerable<T> enumerable, Action<T> action)
    {
        foreach (var item in enumerable) action(item);
    }

    public static TValue GetValueOrDefault<TKey, TValue>(
        this IDictionary<TKey, TValue> dictionary,
        TKey key,
        TValue defaultValue = default)
    {
        return dictionary.TryGetValue(key, out var value) ? value : defaultValue;
    }

    public static IEnumerable<(T item, int index)> WithIndex<T>(this IEnumerable<T> enumerable)
    {
        var i = 0;
        foreach (var item in enumerable)
        {
            yield return (item, i++);
        }
    }
}
}
