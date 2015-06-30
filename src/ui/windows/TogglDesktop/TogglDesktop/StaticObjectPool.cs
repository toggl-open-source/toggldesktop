using System.Collections.Generic;

namespace TogglDesktop
{
    static class StaticObjectPool<T>
    {
        private static readonly Stack<T> pool = new Stack<T>();

        public static void Push(T obj)
        {
            lock (pool)
            {
                //pool.Push(obj);
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

        public static T PopOrDefault()
        {
            lock (pool)
            {
                if (pool.Count > 0)
                    return pool.Pop();
            }
            return default(T);
        }
    }
}