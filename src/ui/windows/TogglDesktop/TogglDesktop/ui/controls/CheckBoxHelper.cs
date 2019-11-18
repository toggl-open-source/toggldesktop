using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace TogglDesktop
{
    public static class CheckBoxHelper
    {
        public static readonly DependencyProperty CheckBoxSizeProperty = DependencyProperty.RegisterAttached("CheckBoxSize", typeof(double), typeof(CheckBoxHelper), new FrameworkPropertyMetadata(18.0));

        /// <summary>
        /// Gets the size of the CheckBox itself.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static double GetCheckBoxSize(DependencyObject obj)
        {
            return (double)obj.GetValue(CheckBoxSizeProperty);
        }

        /// <summary>
        /// Sets the size of the CheckBox itself.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetCheckBoxSize(DependencyObject obj, double value)
        {
            obj.SetValue(CheckBoxSizeProperty, value);
        }


        #region Checked
        // Using a DependencyProperty as the backing store for CheckedGlyph.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty CheckedGlyphProperty = DependencyProperty.RegisterAttached("CheckedGlyph", typeof(object), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty CheckedGlyphTemplateProperty = DependencyProperty.RegisterAttached("CheckedGlyphTemplate", typeof(DataTemplate), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty CheckedBackgroundBrushProperty = DependencyProperty.RegisterAttached("CheckedBackgroundBrush", typeof(Brush), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty CheckedBorderBrushProperty = DependencyProperty.RegisterAttached("CheckedBorderBrush", typeof(Brush), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());


        /// <summary>
        /// Gets the the Glyph for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static object GetCheckedGlyph(DependencyObject obj)
        {
            return (object)obj.GetValue(CheckedGlyphProperty);
        }

        /// <summary>
        /// Sets the the Glyph for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetCheckedGlyph(DependencyObject obj, object value)
        {
            obj.SetValue(CheckedGlyphProperty, value);
        }


        /// <summary>
        /// Gets the the Background for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static Brush GetCheckedBackgroundBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(CheckedBackgroundBrushProperty);
        }

        /// <summary>
        /// Sets the the Background for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetCheckedBackgroundBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(CheckedBackgroundBrushProperty, value);
        }

        /// <summary>
        /// Gets the the GlyphTemplate for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static DataTemplate GetCheckedGlyphTemplate(DependencyObject obj)
        {
            return (DataTemplate)obj.GetValue(CheckedGlyphTemplateProperty);
        }

        /// <summary>
        /// Sets the the GlyphTemplate for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetCheckedGlyphTemplate(DependencyObject obj, DataTemplate value)
        {
            obj.SetValue(CheckedGlyphTemplateProperty, value);
        }

        /// <summary>
        /// Gets the the BorderBrush for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static Brush GetCheckedBorderBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(CheckedBorderBrushProperty);
        }

        /// <summary>
        /// Sets the the BorderBrush for IsChecked = true.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetCheckedBorderBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(CheckedBorderBrushProperty, value);
        }

        #endregion


        #region UnChecked
        // Using a DependencyProperty as the backing store for UnCheckedGlyph.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty UnCheckedGlyphProperty = DependencyProperty.RegisterAttached("UnCheckedGlyph", typeof(object), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty UnCheckedGlyphTemplateProperty = DependencyProperty.RegisterAttached("UnCheckedGlyphTemplate", typeof(DataTemplate), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty UnCheckedBackgroundBrushProperty = DependencyProperty.RegisterAttached("UnCheckedBackgroundBrush", typeof(Brush), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty UnCheckedBorderBrushProperty = DependencyProperty.RegisterAttached("UnCheckedBorderBrush", typeof(Brush), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());

        /// <summary>
        /// Gets the the Glyph for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static object GetUnCheckedGlyph(DependencyObject obj)
        {
            return (object)obj.GetValue(UnCheckedGlyphProperty);
        }

        /// <summary>
        /// Sets the the Glyph for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetUnCheckedGlyph(DependencyObject obj, object value)
        {
            obj.SetValue(UnCheckedGlyphProperty, value);
        }

        /// <summary>
        /// Gets the the BackgroundBrush for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static Brush GetUnCheckedBackgroundBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(UnCheckedBackgroundBrushProperty);
        }

        /// <summary>
        /// Sets the the BackgroundBrush for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetUnCheckedBackgroundBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(UnCheckedBackgroundBrushProperty, value);
        }

        /// <summary>
        /// Gets the the GlyphTemplate for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static DataTemplate GetUnCheckedGlyphTemplate(DependencyObject obj)
        {
            return (DataTemplate)obj.GetValue(UnCheckedGlyphTemplateProperty);
        }

        /// <summary>
        /// Sets the the GlyphTemplate for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetUnCheckedGlyphTemplate(DependencyObject obj, DataTemplate value)
        {
            obj.SetValue(UnCheckedGlyphTemplateProperty, value);
        }

        /// <summary>
        /// Gets the the BorderBrush for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static Brush GetUnCheckedBorderBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(UnCheckedBorderBrushProperty);
        }

        /// <summary>
        /// Sets the the BorderBrush for IsChecked = false.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetUnCheckedBorderBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(UnCheckedBorderBrushProperty, value);
        }

        #endregion


        #region Intermediate
        // Using a DependencyProperty as the backing store for IntermediateGlyph.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty IntermediateGlyphProperty = DependencyProperty.RegisterAttached("IntermediateGlyph", typeof(object), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty IntermediateGlyphTemplateProperty = DependencyProperty.RegisterAttached("IntermediateGlyphTemplate", typeof(DataTemplate), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty IntermediateBackgroundBrushProperty = DependencyProperty.RegisterAttached("IntermediateBackgroundBrush", typeof(Brush), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());
        public static readonly DependencyProperty IntermediateBorderBrushProperty = DependencyProperty.RegisterAttached("IntermediateBorderBrush", typeof(Brush), typeof(CheckBoxHelper), new FrameworkPropertyMetadata());


        /// <summary>
        /// Gets the the Glyph for IsChecked = null.
        /// </summary>
        
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static object GetIntermediateGlyph(DependencyObject obj)
        {
            return (object)obj.GetValue(IntermediateGlyphProperty);
        }

        /// <summary>
        /// Sets the the Glyph for IsChecked = null.
        /// </summary>
        
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetIntermediateGlyph(DependencyObject obj, object value)
        {
            obj.SetValue(IntermediateGlyphProperty, value);
        }


        /// <summary>
        /// Gets the the BackgroundBrush for IsChecked = null.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static Brush GetIntermediateBackgroundBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(IntermediateBackgroundBrushProperty);
        }

        /// <summary>
        /// Sets the the BackgroundBrush for IsChecked = null.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetIntermediateBackgroundBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(IntermediateBackgroundBrushProperty, value);
        }

        /// <summary>
        /// Gets the the GlyphTemplate for IsChecked = null.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static DataTemplate GetIntermediateGlyphTemplate(DependencyObject obj)
        {
            return (DataTemplate)obj.GetValue(IntermediateGlyphTemplateProperty);
        }

        /// <summary>
        /// Sets the the GlyphTemplate for IsChecked = null.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetIntermediateGlyphTemplate(DependencyObject obj, DataTemplate value)
        {
            obj.SetValue(IntermediateGlyphTemplateProperty, value);
        }

        /// <summary>
        /// Gets the the BorderBrush for IsChecked = null.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static Brush GetIntermediateBorderBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(IntermediateBorderBrushProperty);
        }

        /// <summary>
        /// Sets the the BorderBrush for IsChecked = null.
        /// </summary>
        [AttachedPropertyBrowsableForType(typeof(CheckBox))]
        public static void SetIntermediateBorderBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(IntermediateBorderBrushProperty, value);
        }

        #endregion

    }
}