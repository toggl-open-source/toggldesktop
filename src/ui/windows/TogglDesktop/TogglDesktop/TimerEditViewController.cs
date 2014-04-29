using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class TimerEditViewController : UserControl
    {
        public TimerEditViewController()
        {
            InitializeComponent();
        }

        private void comboBoxDescription_SelectedIndexChanged(object sender, EventArgs e)
        {
            // FIXME: apply autocomplete selection
        }

        private void textBoxDuration_TextChanged(object sender, EventArgs e)
        {
            // FIXME: apply duration change
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (buttonStart.Text == "Start") {
                Kopsik.Start(comboBoxDescription.Text,
                    textBoxDuration.Text,
                    0,
                    0);
                return;
            }
            Kopsik.Stop();
        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = buttonStart;
        }
    }
}
