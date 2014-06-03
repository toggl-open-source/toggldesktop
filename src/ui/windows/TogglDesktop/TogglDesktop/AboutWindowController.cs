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
    }
}
