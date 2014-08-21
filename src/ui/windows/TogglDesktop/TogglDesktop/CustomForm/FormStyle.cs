using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using System.Drawing;
using System.Diagnostics;
using System.Xml.Serialization;
using System.IO;
using System.Xml;
using System.Collections.ObjectModel;
using System.Windows.Forms;

namespace TogglDesktop
{

    public static class FormStyleManager
    {
        static FormStyleLibrary globalStyleLibrary;

        public static void Save(string fileName)
        {
            using (Stream stream = File.Create(fileName))
            {
                Save(stream);
            }
        }

        public static void Save(Stream stream)
        {
            if (globalStyleLibrary != null)
                globalStyleLibrary.Save(stream);
        }


        public static void Load(string fileName)
        {
            using (Stream stream = File.OpenRead(fileName))
            {
                Load(stream);
            }
        }

        public static void Load(Stream stream)
        {
            FormStyleLibrary newLibrary = FormStyleLibrary.Load(stream);
            LoadHelper(newLibrary);
        }

        public static void Reset()
        {
            LoadHelper(null);
        }

        public static EventHandler StyleChanged;

        private static void OnStyleChanged()
        {
            if (StyleChanged != null)
                StyleChanged(null, EventArgs.Empty);
        }

        private static void LoadHelper(FormStyleLibrary newLibrary)
        {

            if (newLibrary != globalStyleLibrary)
            {
                FormStyleLibrary oldLibrary = globalStyleLibrary;
                globalStyleLibrary = newLibrary;
                OnStyleChanged();
            }
        }

        internal static FormStyle GetStyle(string styleName)
        {
            if (globalStyleLibrary != null)
            {
                foreach (FormStyle style in globalStyleLibrary.Styles)
                    if (styleName == style.Name)
                        return style;
            }
            
            return null;
        }

        internal static FormStyle GetDefaultStyle()
        {
            if (globalStyleLibrary != null)
                return GetStyle(globalStyleLibrary.DefaultStyleName);
            
            return null;
        }

        internal static string[] GetStyleNames()
        {
            List<string> styleNames = new List<string>();
            if (globalStyleLibrary != null)
            {
                foreach (FormStyle style in globalStyleLibrary.Styles)
                    styleNames.Add(style.Name);
            }
            return styleNames.ToArray();
        }

        internal static FormStyle AddNewStyle()
        {
            FormStyle style = new FormStyle();

            if (globalStyleLibrary == null)
                globalStyleLibrary = new FormStyleLibrary();

            List<string> styleNames = new List<string>(FormStyleManager.GetStyleNames());
            style.Name = "FormStyle";
            for (int i = 1; styleNames.Contains(style.Name); i++)
                style.Name = String.Format("FormStyle{0}", i);

            globalStyleLibrary.Styles.Add(style);
            OnStyleChanged();

            return style;
        }

        internal static void DeleteStyle(FormStyle style)
        {
            if (style == null)
                throw new ArgumentNullException("style");

            if (globalStyleLibrary == null)
                return;

            globalStyleLibrary.Styles.Remove(style);
            OnStyleChanged();
        }
    }

    public abstract class StyleObject : INotifyPropertyChanged
    {
        private StyleObject _parent;

        [XmlIgnore]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public StyleObject Parent
        {
            get { return _parent; }
            set { _parent = value; }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));

            OnChildPropertyChanged(this, propertyName);
        }

        public event ChildPropertyChangedEventHandler ChildPropertyChanged;

        protected virtual void OnChildPropertyChanged(StyleObject subObject, string propertyName)
        {
            if (ChildPropertyChanged != null)
                ChildPropertyChanged(this, new ChildPropertyChangedEventArgs(subObject, propertyName));

            if (Parent != null)
                Parent.OnChildPropertyChanged(subObject, propertyName);
        }
    }

    public delegate void ChildPropertyChangedEventHandler(object sender, ChildPropertyChangedEventArgs args);

    public class ChildPropertyChangedEventArgs : PropertyChangedEventArgs
    {
        private StyleObject _subObject;
        public StyleObject SubObject
        {
            get { return _subObject; }
        }

        public ChildPropertyChangedEventArgs(StyleObject subObject, string propertyName)
            : base(propertyName)
        {
            _subObject = subObject;
        }
    }

    [System.Xml.Serialization.XmlRoot("styleLibrary")]
    public class FormStyleLibrary : StyleObject
    {
        private List<FormStyle> _styles = new List<FormStyle>();

        [XmlArray("styles")]
        [Description("List of styles contained in this library")]
        public List<FormStyle> Styles
        {
            get { return _styles; }
        }

        private string _version = "1.0";

        [XmlAttribute("schemaVersion")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        [Description("Version of Xml Schema the style library was saved")]
        public string SchemaVersion
        {
            get { return _version; }
            set { _version = value; }
        }


        private string _defaultStyleName = "Default";

        [XmlAttribute("defaultStyleName")]
        [Description("Name of default style in this library")]
        public string DefaultStyleName
        {
            get { return _defaultStyleName; }
            set 
            {
                _defaultStyleName = value; 
            }
        }

        internal static FormStyleLibrary Load(Stream stream)
        {
            using (XmlReader reader = XmlReader.Create(stream))
            {
                XmlSerializer serializer = new XmlSerializer(typeof(FormStyleLibrary));
                FormStyleLibrary library = (FormStyleLibrary)serializer.Deserialize(reader);
                return library;
            }
        }

        internal void Save(Stream stream)
        {
            XmlWriterSettings set = new XmlWriterSettings();
            set.Indent = true;

            using (XmlWriter wr = XmlWriter.Create(stream, set))
            {
                XmlSerializer serializer = new XmlSerializer(typeof(FormStyleLibrary));
                serializer.Serialize(wr, this);
            }
        }
    }

    internal static class FormStyleProperty 
    {
        public const string Name = "Name";
        public const string NormalState = "NormalState";
        public const string ClientAreaPadding = "ClientAreaPadding";
        public const string IconPadding = "IconPadding";
        public const string TitlePadding = "TitlePadding";
        public const string TitleFont = "TitleFont";
        public const string TitleColor = "TitleColor";
        public const string TitleShadowColor = "TitleShadowColor";
        public const string MinimizeButton = "MinimizeButton";
        public const string MaximizeButton = "MaximizeButton";
        public const string CloseButton = "CloseButton";
        public const string RestoreButton = "RestoreButton";
        public const string HelpButton = "HelpButton";
        public const string SizingBorderWidth = "SizingBorderWidth";
        public const string SizingCornerOffset = "SizingCornerOffset";
    }

 #if !DEBUGFORM
	[DebuggerStepThrough]
#endif
    [XmlType("formStyle")]
    public sealed class FormStyle : StyleObject
	{
        public FormStyle()
        {
            NormalState = new SerializableImage();
            MinimizeButton = new FormButtonStyle();
		    MaximizeButton = new FormButtonStyle();
		    CloseButton = new FormButtonStyle();
		    RestoreButton = new FormButtonStyle();
		    HelpButton = new FormButtonStyle();
        }

        private string _name;

        [XmlAttribute("name")]
        [Description("Name used to indentify this style in the library")]
        public string Name
        {
            get { return _name; }
            set 
            {
                if (String.IsNullOrEmpty(value))
                    throw new ArgumentNullException(FormStyleProperty.Name);

                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(FormStyleProperty.Name);
                }
            }
        }


        private Padding _clientAreaPadding;

        [XmlIgnore]
        [Description("Padding of the client rectangle relative to window bounds")]
        public Padding ClientAreaPadding
        {
            get { return _clientAreaPadding; }
            set {
                if (_clientAreaPadding != value)
                {
                    _clientAreaPadding = value;
                    OnPropertyChanged(FormStyleProperty.ClientAreaPadding);
                }
            }
        }

        [XmlAttribute("clientAreaPadding")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string ClientAreaPadding_XmlSurrogate
        {
            get
            {
                if (ClientAreaPadding != Padding.Empty)
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    return PaddingConverter.ConvertToInvariantString(ClientAreaPadding);
                }
                return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    ClientAreaPadding = (Padding)PaddingConverter.ConvertFromInvariantString(value);
                }
                else
                    ClientAreaPadding = Padding.Empty;
            }
        }

        private Padding _iconPadding;

        [XmlIgnore]
        [Description("Padding of icon rectangle relative to top-left corner of title bar")]
        public Padding IconPadding
        {
            get { return _iconPadding; }
            set
            {
                if (_iconPadding != value)
                {
                    _iconPadding = value;
                    OnPropertyChanged(FormStyleProperty.IconPadding);
                }
            }
        }

        [XmlAttribute("iconPadding")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string IconPadding_XmlSurrogate
        {
            get
            {
                if (IconPadding != Padding.Empty)
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    return PaddingConverter.ConvertToInvariantString(IconPadding);
                }
                return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    IconPadding = (Padding)PaddingConverter.ConvertFromInvariantString(value);
                }
                else
                    IconPadding = Padding.Empty;
            }
        }

        private Padding _titlePadding;

        [XmlIgnore]
        [Description("Padding of the title rectangle relative to rectangle of title bar")]
        public Padding TitlePadding
        {
            get { return _titlePadding; }
            set
            {
                if (_titlePadding != value)
                {
                    _titlePadding = value;
                    OnPropertyChanged(FormStyleProperty.TitlePadding);
                }
            }
        }

        [XmlAttribute("titlePadding")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string TitlePadding_XmlSurrogate
        {
            get
            {
                if (TitlePadding != Padding.Empty)
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    return PaddingConverter.ConvertToInvariantString(TitlePadding);
                }
                return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    TitlePadding = (Padding)PaddingConverter.ConvertFromInvariantString(value);
                }
                else
                    TitlePadding = Padding.Empty;
            }
        }

        private Font _titleFont;

        [XmlIgnore]
        [Description("Font used to paint window title")]
        public Font TitleFont
        {
	        get { return _titleFont; }
            set
            {
                if (_titleFont != value)
                {
                    _titleFont = value;
                    OnPropertyChanged(FormStyleProperty.TitleFont);
                }
            }
        }

        [XmlAttribute("titleFont")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string TitleFont_XmlSurrogate
        {
            get 
            {
                if (TitleFont != null)
                {
                    TypeConverter FontConverter = TypeDescriptor.GetConverter(typeof(Font));
                    return FontConverter.ConvertToInvariantString(TitleFont);
                } else 
                    return null;
            }
            set 
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter FontConverter = TypeDescriptor.GetConverter(typeof(Font));
                    TitleFont = (Font)FontConverter.ConvertFromInvariantString(value);
                }
                else
                    TitleFont = null;
            }
        }

        private Color _titleColor;

        [XmlIgnore]
        [Description("Color used to paint window title")]
        public Color TitleColor
		{
			get { return _titleColor; }
			set {
                if (_titleColor != value)
                {
                    _titleColor = value;
                    OnPropertyChanged(FormStyleProperty.TitleColor);
                }
            }
		}

        [XmlAttribute("titleColor")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string TitleColor_XmlSurrogate
        {
            get
            {
                if (TitleColor != Color.Empty)
                {
                    TypeConverter ColorConverter = TypeDescriptor.GetConverter(typeof(Color));
                    return ColorConverter.ConvertToInvariantString(TitleColor);
                }
                else
                    return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter ColorConverter = TypeDescriptor.GetConverter(typeof(Color));
                    TitleColor = (Color)ColorConverter.ConvertFromInvariantString(value);
                }
                else
                    TitleColor = Color.Empty;
            }
        }


        private Color _titleShadowColor;

        [XmlIgnore]
        [Description("Color used to draw drop shadow behind window title")]
        public Color TitleShadowColor
        {
            get { return _titleShadowColor; }
            set
            {
                if (_titleShadowColor != value)
                {
                    _titleShadowColor = value;
                    OnPropertyChanged(FormStyleProperty.TitleShadowColor);
                }
            }
        }

        [XmlAttribute("titleShadowColor")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string TitleShadowColor_XmlSurrogate
        {
            get
            {
                if (TitleShadowColor != Color.Empty)
                {
                    TypeConverter ColorConverter = TypeDescriptor.GetConverter(typeof(Color));
                    return ColorConverter.ConvertToInvariantString(TitleShadowColor);
                }
                else
                    return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter ColorConverter = TypeDescriptor.GetConverter(typeof(Color));
                    TitleShadowColor = (Color)ColorConverter.ConvertFromInvariantString(value);
                }
                else
                    TitleShadowColor = Color.Empty;
            }
        }


        private int _sizingBorderWidth = 3;

        [XmlAttribute("sizingBorderWidth")]
        [Description("Offset from window border where window can be sized using a mouse horizontaly or verically")]
        public int SizingBorderWidth
        {
            get { return _sizingBorderWidth; }
            set
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException("SizingBorderWidth", "Sizing offset must be greater then zero.");

                if (_sizingBorderWidth != value)
                {
                    _sizingBorderWidth = value;
                    OnPropertyChanged(FormStyleProperty.SizingBorderWidth);
                }
            }
        }

        private int _sizingCornerOffset = 16;

        [XmlAttribute("sizingBorderOffset")]
        [Description("Offset from window corner where form can be sized in both directions")]
        public int SizingCornerOffset
        {
            get { return _sizingCornerOffset; }
            set
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException("SizingCornerOffset", "Sizing offset must be greater then zero.");

                if (_sizingCornerOffset != value)
                {
                    _sizingCornerOffset = value;
                    OnPropertyChanged(FormStyleProperty.SizingCornerOffset);
                }
            }
        }

        private SerializableImage _normalState;

        [XmlElement("normalState")]
        [Browsable(false)]
        [Description("Image used to paint background on forms non client area")]
        public SerializableImage NormalState
        {
            get { return _normalState; }
            set
            {
                if (value == null)
                    throw new ArgumentNullException("NormalState");

                if (_normalState != value)
                {
                    _normalState = value;
                    _normalState.Parent = this;
                    OnPropertyChanged(FormStyleProperty.NormalState);
                }
            }
        }

        private FormButtonStyle _minimizeButton;

        [XmlElement("minimizeButton")]
        [Browsable(false)]
        [Description("Style for window minimize button")]
        public FormButtonStyle MinimizeButton
		{
			get { return _minimizeButton; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException("MinimizeButton");

                if (_minimizeButton != value)
                {
                    _minimizeButton = value;
                    _minimizeButton.Parent = this;
                    OnPropertyChanged(FormStyleProperty.MinimizeButton);
                }
            }
		}

        private FormButtonStyle _maximizeButton;

        [XmlElement("maximizeButton")]
        [Browsable(false)]
        [Description("Style for window maximize button")]
        public FormButtonStyle MaximizeButton
		{
			get { return _maximizeButton; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException("MaximizeButton");

                if (_maximizeButton != value)
                {
                    _maximizeButton = value;
                    _maximizeButton.Parent = this;
                    OnPropertyChanged(FormStyleProperty.MaximizeButton);
                }
            }
		}

        private FormButtonStyle _closeButton;

        [XmlElement("closeButton")]
        [Browsable(false)]
        [Description("Style for window close button")]
        public FormButtonStyle CloseButton
		{
			get { return _closeButton; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException("CloseButton");

                if (_closeButton != value)
                {
                    _closeButton = value;
                    _closeButton.Parent = this;
                    OnPropertyChanged(FormStyleProperty.CloseButton);
                }
            }
		}

        private FormButtonStyle _restoreButton;

        [XmlElement("restoreButton")]
        [Browsable(false)]
        [Description("Style for window restore button")]
        public FormButtonStyle RestoreButton
		{
			get { return _restoreButton; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException("RestoreButton");

                if (_restoreButton != value)
                {
                    _restoreButton = value;
                    _restoreButton.Parent = this;
                    OnPropertyChanged(FormStyleProperty.RestoreButton);
                }
            }
		}

        private FormButtonStyle _helpButton;

        [XmlElement("helpButton")]
        [Browsable(false)]
        [Description("Style for window help button")]
        public FormButtonStyle HelpButton
		{
			get { return _helpButton; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException("HelpButton");

                if (_helpButton != value)
                {
                    _helpButton = value;
                    _helpButton.Parent = this;
                    OnPropertyChanged(FormStyleProperty.HelpButton);
                }
            } 
		}
    }

    internal static class FormButtonStyleProperty
    {
        public const string Size = "Size";
        public const string Margin = "Margin";
        public const string NormalState = "NormalState";
        public const string ActiveState = "ActiveState";
        public const string HoverState = "HoverState";
        public const string DisabledState = "DisabledState";
    }

#if !DEBUGFORM
    [DebuggerStepThrough]
#endif
    [XmlType("buttonStyle")]
    public class FormButtonStyle : StyleObject
    {

        public FormButtonStyle()
        {
            NormalState = new SerializableImage();
            ActiveState = new SerializableImage();
            HoverState = new SerializableImage();
            DisabledState = new SerializableImage();
        }

        private Size _size;

        [XmlIgnore]
        [Description("Size of button rectangle.")]
        public Size Size
        {
            get { return _size; }
            set 
            {
                if (_size != value)
                {
                    _size = value;
                    OnPropertyChanged(FormButtonStyleProperty.Size);
                }
            }
        }

        [XmlAttribute("size")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string Size_XmlSurrogate
        {
            get 
            {
                if (Size != Size.Empty)
                {
                    TypeConverter SizeConverter = TypeDescriptor.GetConverter(typeof(Size));
                    return SizeConverter.ConvertToInvariantString(Size);
                }
                return null;
            }
            set 
            { 
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter SizeConverter = TypeDescriptor.GetConverter(typeof(Size));
                    Size = (Size)SizeConverter.ConvertFromInvariantString(value);
                }
                else
                    Size = Size.Empty;
            }
        }

        private Padding _margin;

        [XmlIgnore]
        [Description("Margin around button rectangle relative to title bar")]
        public Padding Margin
        {
            get { return _margin; }
            set 
            {
                if (_margin != value)
                {
                    _margin = value;
                    OnPropertyChanged(FormButtonStyleProperty.Margin);
                }
            }
        }

        [XmlAttribute("margin")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string Margin_XmlSurrogate
        {
            get
            {
                if (Margin != Padding.Empty)
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    return PaddingConverter.ConvertToInvariantString(Margin);
                }
                return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    Margin = (Padding)PaddingConverter.ConvertFromInvariantString(value);
                }
                else
                    Margin = Padding.Empty;
            }
        }

        private SerializableImage _normalState;

        [Browsable(false)]
        [XmlElement("normalState")]
        [Description("Image used to paint button in normal state")]
        public SerializableImage NormalState
        {
            get { return _normalState; }
            set {
                if (value == null)
                    throw new ArgumentNullException(FormButtonStyleProperty.NormalState);

                if (_normalState != value)
                {
                    _normalState = value;
                    _normalState.Parent = this;
                    OnPropertyChanged(FormButtonStyleProperty.NormalState);
                }
            }
        }

        private SerializableImage _activeState;

        [Browsable(false)]
        [XmlElement("activeState")]
        [Description("Image used to paint button in active state")]
        public SerializableImage ActiveState
        {
            get { return _activeState; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException(FormButtonStyleProperty.ActiveState);

                if (_activeState != value)
                {
                    _activeState = value;
                    _activeState.Parent = this;
                    OnPropertyChanged(FormButtonStyleProperty.ActiveState);
                }
            }
        }

        private SerializableImage _hoverState;

        [Browsable(false)]
        [XmlElement("hoverState")]
        [Description("Image used to paint button in hover state")]
        public SerializableImage HoverState
        {
            get { return _hoverState; }
            set 
            {
                if (value == null)
                    throw new ArgumentNullException(FormButtonStyleProperty.HoverState);

                if (_hoverState != value)
                {
                    _hoverState = value;
                    _hoverState.Parent = this;
                    OnPropertyChanged(FormButtonStyleProperty.HoverState);
                }
            }
        }

        private SerializableImage _disabledState;

        [Browsable(false)]
        [XmlElement("disabledState")]
        [Description("Image used to paint button in disabled state")]
        public SerializableImage DisabledState
        {
            get { return _disabledState; }
            set
            {
                if (value == null)
                    throw new ArgumentNullException(FormButtonStyleProperty.DisabledState);

                if (_disabledState != value)
                {
                    _disabledState = value;
                    _disabledState.Parent = this;
                    OnPropertyChanged(FormButtonStyleProperty.DisabledState);
                }
            }
        }
    }

    internal static class SerializableImageProperty
    {
        public const string SizeMode = "SizeMode";
        public const string StretchMargins = "StretchMargins";
        public const string Image = "Image";
    }

    public sealed class SerializableImage : StyleObject
    {
        private ImageSizeMode _sizeMode;

        [XmlAttribute("sizeMode")]
        public ImageSizeMode SizeMode
        {
            get { return _sizeMode; }
            set {
                if (_sizeMode != value)
                {
                    _sizeMode = value;
                    OnPropertyChanged(SerializableImageProperty.SizeMode);
                }
            }
        }

        private Padding _stretchMargins;

        [XmlIgnore]
        public Padding StretchMargins
        {
            get { return _stretchMargins; }
            set 
            {
                if (_stretchMargins != value)
                {
                    _stretchMargins = value;
                    OnPropertyChanged(SerializableImageProperty.StretchMargins);
                }
            }
        }

        [XmlAttribute("stretchMargins")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public string StretchMargins_XmlSurrogate
        {
            get
            {
                if (StretchMargins != Padding.Empty)
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    return PaddingConverter.ConvertToInvariantString(StretchMargins);
                }
                return null;
            }
            set
            {
                if (!String.IsNullOrEmpty(value))
                {
                    TypeConverter PaddingConverter = TypeDescriptor.GetConverter(typeof(Padding));
                    StretchMargins = (Padding)PaddingConverter.ConvertFromInvariantString(value);
                }
                else
                    StretchMargins = Padding.Empty;
            }
        }

        private Bitmap _image;

        [XmlIgnore]
        public Bitmap Image
        {
            get { return _image; }
            set 
            {
                if (_image != value)
                {
                    _image = value;
                    OnPropertyChanged(SerializableImageProperty.Image);
                }
            }
        }

        [XmlElement("image")]
        [Browsable(false)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public byte[] Image_XmlSurrogate
        {
            get
            {
                if (Image != null)
                {
                    TypeConverter BitmapConverter =
                        TypeDescriptor.GetConverter(Image.GetType());
                    return (byte[])BitmapConverter.ConvertTo(Image, typeof(byte[]));
                }
                else
                    return null;
            }
            set
            {
                if (value != null)
                    Image = new Bitmap(new MemoryStream(value));
                else
                    Image = null; 
            }
        }

        public void DrawImage(Graphics g, Rectangle destRect)
        {
            if (Image == null)
                return;

            DrawUtil.DrawImage(g, Image, destRect, SizeMode, StretchMargins);
        }
    }
}
