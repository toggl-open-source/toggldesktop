using System;
using System.Windows.Controls;

namespace TogglDesktop
{
static class UIExtensions
{
    public static bool TryBeginInvoke(this System.Windows.Forms.Control control, Action action)
    {
        if (!control.InvokeRequired)
            return false;

        control.BeginInvoke(action);

        return true;
    }

    #region overloads

    public static bool TryBeginInvoke<T0>(this System.Windows.Forms.Control control,
                                          Action<T0> action, T0 p0)
    {
        return control.TryBeginInvoke(() => action(p0));
    }
    public static bool TryBeginInvoke<T0, T1>(this System.Windows.Forms.Control control,
            Action<T0, T1> action, T0 p0, T1 p1)
    {
        return control.TryBeginInvoke(() => action(p0, p1));
    }
    public static bool TryBeginInvoke<T0, T1, T2>(this System.Windows.Forms.Control control,
            Action<T0, T1, T2> action, T0 p0, T1 p1, T2 p2)
    {
        return control.TryBeginInvoke(() => action(p0, p1, p2));
    }
    public static bool TryBeginInvoke<T0, T1, T2, T3>(this System.Windows.Forms.Control control,
            Action<T0, T1, T2, T3> action, T0 p0, T1 p1, T2 p2, T3 p3)
    {
        return control.TryBeginInvoke(() => action(p0, p1, p2, p3));
    }
    public static bool TryBeginInvoke<T0, T1, T2, T3, T4>(this System.Windows.Forms.Control control,
            Action<T0, T1, T2, T3, T4> action, T0 p0, T1 p1, T2 p2, T3 p3, T4 p4)
    {
        return control.TryBeginInvoke(() => action(p0, p1, p2, p3, p4));
    }

    #endregion


    public static bool TryBeginInvoke(this Control control, Action action)
    {
        if (control.Dispatcher.CheckAccess())
            return false;

        control.Dispatcher.BeginInvoke(action);

        return true;
    }

    #region overloads

    public static bool TryBeginInvoke<T0>(this Control control, Action<T0> action, T0 p0)
    {
        return control.TryBeginInvoke(() => action(p0));
    }
    public static bool TryBeginInvoke<T0, T1>(this Control control, Action<T0, T1> action, T0 p0, T1 p1)
    {
        return control.TryBeginInvoke(() => action(p0, p1));
    }
    public static bool TryBeginInvoke<T0, T1, T2>(this Control control, Action<T0, T1, T2> action, T0 p0, T1 p1, T2 p2)
    {
        return control.TryBeginInvoke(() => action(p0, p1, p2));
    }
    public static bool TryBeginInvoke<T0, T1, T2, T3>(this Control control,
            Action<T0, T1, T2, T3> action, T0 p0, T1 p1, T2 p2, T3 p3)
    {
        return control.TryBeginInvoke(() => action(p0, p1, p2, p3));
    }
    public static bool TryBeginInvoke<T0, T1, T2, T3, T4>(this Control control,
            Action<T0, T1, T2, T3, T4> action, T0 p0, T1 p1, T2 p2, T3 p3, T4 p4)
    {
        return control.TryBeginInvoke(() => action(p0, p1, p2, p3, p4));
    }

    #endregion
}
}