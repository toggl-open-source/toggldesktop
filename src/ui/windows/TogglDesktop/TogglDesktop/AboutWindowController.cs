using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class AboutWindowController : Form
    {
        public AboutWindowController()
        {
            InitializeComponent();
            KopsikApi.OnUpdate += OnUpdate;
        }

        void OnUpdate(bool open, ref KopsikApi.KopsikUpdateViewItem view)
        {
            DisplayUpdate(open, view);
        }

        void DisplayUpdate(bool open, KopsikApi.KopsikUpdateViewItem view)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayUpdate(open, view); });
                return;
            }
            comboBoxChannel.Tag = "ignore";
            try
            {
                comboBoxChannel.Text = view.UpdateChannel;
            }
            finally
            {
                comboBoxChannel.Tag = null;
            }
        }

        private void AboutWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();
            e.Cancel = true;
        }

        private void buttonCheckingForUpdate_Click(object sender, EventArgs e)
        {

        }

        private void AboutWindowController_Load(object sender, EventArgs e)
        {

        }

        private void comboBoxChannel_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (null == comboBoxChannel.Tag)
            {
                KopsikApi.kopsik_set_update_channel(KopsikApi.ctx, comboBoxChannel.Text);
            }
        }
    }
}
