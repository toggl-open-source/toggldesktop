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
            
            style.NormalState.Image = TogglDesktop.Properties.Resources.Border;
            style.NormalState.SizeMode = ImageSizeMode.Tiled;
            style.NormalState.StretchMargins = new Padding(3, 30, 3, 3);

            style.CloseButton.Size = TogglDesktop.Properties.Resources.Close.Size;
            style.CloseButton.Margin = new Padding(0, 3, 0, 0);
            style.CloseButton.NormalState.Image = TogglDesktop.Properties.Resources.Close;
            style.CloseButton.DisabledState.Image = TogglDesktop.Properties.Resources.CloseDisabled;
            style.CloseButton.ActiveState.Image = TogglDesktop.Properties.Resources.ClosePressed;
            style.CloseButton.HoverState.Image = TogglDesktop.Properties.Resources.CloseHot;

            style.MinimizeButton.Size = TogglDesktop.Properties.Resources.Minimize.Size;
            style.MinimizeButton.Margin = new Padding(0, 3, 0, 0); ;
            style.MinimizeButton.NormalState.Image = TogglDesktop.Properties.Resources.Minimize;
            style.MinimizeButton.DisabledState.Image = TogglDesktop.Properties.Resources.MinimizeDisabled;
            style.MinimizeButton.ActiveState.Image = TogglDesktop.Properties.Resources.MinimizePressed;
            style.MinimizeButton.HoverState.Image = TogglDesktop.Properties.Resources.MinimizeHot;

            style.HelpButton.Size = TogglDesktop.Properties.Resources.cog.Size;
            style.HelpButton.Margin = new Padding(0, 3, 0, 0); ;
            style.HelpButton.NormalState.Image = TogglDesktop.Properties.Resources.cog;
            style.HelpButton.DisabledState.Image = TogglDesktop.Properties.Resources.cogDisabled;
            style.HelpButton.ActiveState.Image = TogglDesktop.Properties.Resources.cogPressed;
            style.HelpButton.HoverState.Image = TogglDesktop.Properties.Resources.cogHot;

            style.TitleColor = Color.White;
            style.TitleFont = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular);

            style.ClientAreaPadding = new Padding(0, 30, 0, 0);
            style.IconPadding = new Padding(7, 6, 10, 0);

            this.NonClientAreaDoubleBuffering = true;

            return style;
        }
    }
}
