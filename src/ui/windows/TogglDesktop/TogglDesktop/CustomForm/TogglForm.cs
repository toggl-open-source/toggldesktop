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

namespace Kobush.Windows.Forms
{
    public class TogglForm : CustomBorderForm
    {
        public TogglForm()
        {
            this.FormStyle = CreateFormStyle();
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            /*
            int diam = 10;
            GraphicsPath path = new GraphicsPath();
            path.AddArc(0, 0, diam, diam, -90, -90);
            path.AddLines(new Point[] {new Point(0,diam), new Point(0, Height), 
                new Point(Width, Height), new Point(Width, diam)});
            path.AddArc(Width - diam, 0, diam, diam, 0, -90);
            path.CloseFigure();

            this.Region = new Region(path);
            */
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

            style.MaximizeButton.Size = TogglDesktop.Properties.Resources.Maximize.Size;
            style.MaximizeButton.Margin = new Padding(0, 3, 0, 0); ;
            style.MaximizeButton.NormalState.Image = TogglDesktop.Properties.Resources.Maximize;
            style.MaximizeButton.DisabledState.Image = TogglDesktop.Properties.Resources.MaximizeDisabled;
            style.MaximizeButton.ActiveState.Image = TogglDesktop.Properties.Resources.MaximizePressed;
            style.MaximizeButton.HoverState.Image = TogglDesktop.Properties.Resources.MaximizeHot;

            style.MinimizeButton.Size = TogglDesktop.Properties.Resources.Minimize.Size;
            style.MinimizeButton.Margin = new Padding(0, 3, 0, 0); ;
            style.MinimizeButton.NormalState.Image = TogglDesktop.Properties.Resources.Minimize;
            style.MinimizeButton.DisabledState.Image = TogglDesktop.Properties.Resources.MinimizeDisabled;
            style.MinimizeButton.ActiveState.Image = TogglDesktop.Properties.Resources.MinimizePressed;
            style.MinimizeButton.HoverState.Image = TogglDesktop.Properties.Resources.MinimizeHot;
/*
            style.RestoreButton.Size = Properties.Resources.Restore.Size;
            style.RestoreButton.Margin = new Padding(1, 5, 1, 0);
            style.RestoreButton.NormalState.Image = Properties.Resources.Restore;
            style.RestoreButton.DisabledState.Image = Properties.Resources.RestoreDisabled;
            style.RestoreButton.ActiveState.Image = Properties.Resources.RestorePressed;
            style.RestoreButton.HoverState.Image = Properties.Resources.RestoreHot;
             * */

            style.TitleColor = Color.White;
            //style.TitleShadowColor = Color.DimGray;
            style.TitleFont = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular);

            style.ClientAreaPadding = new Padding(0, 30, 0, 1);
            style.IconPadding = new Padding(7, 7, 10, 0);

            this.NonClientAreaDoubleBuffering = true;

            return style;
        }
    }
}
