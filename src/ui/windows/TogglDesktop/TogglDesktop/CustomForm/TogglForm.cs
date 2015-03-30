#region Custom Border Forms - Copyright (C) 2005 Szymon Kobalczyk

// Custom Border Forms
// Copyright (C) 2005 Szymon Kobalczyk
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Szymon Kobalczyk (http://www.geekswithblogs.com/kobush)

#endregion

#region Using directives

using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

#endregion

namespace TogglDesktop
{
    public class TogglForm : CustomBorderForm
    {
        public TogglForm()
        {
            this.FormStyle = CreateFormStyle();
        }

        private const int CS_DROPSHADOW = 0x20000;
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cp = base.CreateParams;
                cp.ClassStyle |= CS_DROPSHADOW;
                return cp;
            }
        }

        FormStyle CreateFormStyle()
        {
            FormStyle style = new FormStyle();
            
            style.NormalState.Image = TogglDesktop.Properties.Resources.darkBg;
            style.NormalState.SizeMode = ImageSizeMode.Tiled;
            style.NormalState.StretchMargins = new Padding(3, 30, 3, 3);

            style.CloseButton.Size = TogglDesktop.Properties.Resources.Close.Size;
            style.CloseButton.Margin = new Padding(0, 16, 8, 0);
            style.CloseButton.NormalState.Image = TogglDesktop.Properties.Resources.Close;
            style.CloseButton.DisabledState.Image = TogglDesktop.Properties.Resources.CloseDisabled;
            style.CloseButton.ActiveState.Image = TogglDesktop.Properties.Resources.CloseHot;
            style.CloseButton.HoverState.Image = TogglDesktop.Properties.Resources.CloseHot;

            style.MinimizeButton.Size = TogglDesktop.Properties.Resources.Minimize.Size;
            style.MinimizeButton.Margin = new Padding(0, 13, 16, 0);
            style.MinimizeButton.NormalState.Image = TogglDesktop.Properties.Resources.Minimize;
            style.MinimizeButton.DisabledState.Image = TogglDesktop.Properties.Resources.MinimizeDisabled;
            style.MinimizeButton.ActiveState.Image = TogglDesktop.Properties.Resources.MinimizeHot;
            style.MinimizeButton.HoverState.Image = TogglDesktop.Properties.Resources.MinimizeHot;

            style.HelpButton.Size = TogglDesktop.Properties.Resources.cog.Size;
            style.HelpButton.Margin = new Padding(0, 14, 14, 0);
            style.HelpButton.NormalState.Image = TogglDesktop.Properties.Resources.cog;
            style.HelpButton.DisabledState.Image = TogglDesktop.Properties.Resources.cog;
            style.HelpButton.ActiveState.Image = TogglDesktop.Properties.Resources.cogHot;
            style.HelpButton.HoverState.Image = TogglDesktop.Properties.Resources.cogHot;

            Font f = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular);
            style.TitleColor = Color.White;
            style.TitleFont = f;
            style.ClientAreaPadding = new Padding(0, 40, 0, 0);
            style.IconPadding = new Padding(11, 11, 10, 0);
            style.TitlePadding = new Padding(5, 0, 0, 0);
            NonClientAreaDoubleBuffering = true;

            return style;
        }


        // Custom ttf Font loading
        private static PrivateFontCollection s_FontCollection = new PrivateFontCollection();

        public static FontFamily[] FontFamilies
        {
            get
            {
                if (s_FontCollection.Families.Length == 0)
                    LoadFonts();

                return s_FontCollection.Families;
            }
        }

        public static Font GetFont(String family, Font defaultFont)
        {
            foreach (FontFamily font in FontFamilies)
            {
                if (font.Name.ToLower().Equals(family.ToLower()))
                {
                    return new Font(font, defaultFont.Size, defaultFont.Style);
                }
            }

            return defaultFont;
        }

        public static void ApplyFont(int family, Control control)
        {
            control.Font = new Font(FontFamilies[family], control.Font.Size, control.Font.Style);
        }

        public static void ApplyFont(string family, Control control)
        {
            foreach (FontFamily font in FontFamilies)
            {
                if (font.Name.ToLower().Equals(family.ToLower()))
                {
                    control.Font = new Font(font, control.Font.Size, control.Font.Style);
                }
            }
        }

        public static void LoadFonts()
        {
            if (Assembly.GetEntryAssembly() == null || Assembly.GetEntryAssembly().GetManifestResourceNames() == null)
                return;

            foreach (string resource in Assembly.GetEntryAssembly().GetManifestResourceNames())
            {
                // Load TTF files from your Fonts resource folder.
                if (resource.Contains(".Fonts.") && resource.ToLower().EndsWith(".ttf"))
                {
                    using (Stream stream = Assembly.GetEntryAssembly().GetManifestResourceStream(resource))
                    {
                        try
                        {
                            // create an unsafe memory block for the font data
                            System.IntPtr data = Marshal.AllocCoTaskMem((int)stream.Length);

                            // create a buffer to read in to
                            byte[] fontdata = new byte[stream.Length];

                            // read the font data from the resource
                            stream.Read(fontdata, 0, (int)stream.Length);

                            // copy the bytes to the unsafe memory block
                            Marshal.Copy(fontdata, 0, data, (int)stream.Length);

                            // pass the font to the font collection
                            s_FontCollection.AddMemoryFont(data, (int)stream.Length);

                            // close the resource stream
                            stream.Close();

                            // free up the unsafe memory
                            Marshal.FreeCoTaskMem(data);
                        }
                        catch(Exception e)
                        {
                            System.Console.WriteLine("Error loading font: " + resource.ToLower());
                            System.Console.WriteLine(e.StackTrace);
                        }
                    }
                }
            }
        }

    }
}
