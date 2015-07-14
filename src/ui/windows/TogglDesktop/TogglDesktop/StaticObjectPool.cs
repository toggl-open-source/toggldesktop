using System.Collections.Generic;

namespace TogglDesktop
{
static class StaticObjectPool
{
    private static class Pool<T>
    {
        private static readonly Stack<T> pool = new Stack<T>();

        public static void Push(T obj)
        {
            lock (pool)
            {
                pool.Push(obj);
            }
        }

        public static bool TryPop(out T obj)
        {
            lock (pool)
            {
                if (pool.Count > 0)
                {
                    obj = pool.Pop();
                    return true;
                }
            }
            obj = default(T);
            return false;
        }
    }

    public static void Push<T>(T obj)
    {
        Pool<T>.Push(obj);
    }

    public static bool TryPop<T>(out T obj)
    {
        return Pool<T>.TryPop(out obj);
    }

    public static T PopOrDefault<T>()
    {
        T ret;
        TryPop(out ret);
        return ret;
    }

    public static T PopOrNew<T>()
    where T : new()
    {
        T ret;
        return TryPop(out ret) ? ret : new T();
    }
}
}