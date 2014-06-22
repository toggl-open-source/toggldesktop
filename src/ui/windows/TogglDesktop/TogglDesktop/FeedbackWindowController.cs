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
    public partial class FeedbackWindowController : Form
    {
        public FeedbackWindowController()
        {
            InitializeComponent();
        }

        private void FeedbackWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();
            e.Cancel = true;
        }

        private void FeedbackWindowController_Load(object sender, EventArgs e)
        {
            comboBoxTopic.SelectedIndex = 0;
        }

        private void buttonUploadImage_Click(object sender, EventArgs e)
        {
            openFileDialog.ShowDialog();
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            if (comboBoxTopic.SelectedIndex == 0)
            {
                comboBoxTopic.Focus();
                return;
            }
            if (richTextBoxContents.TextLength == 0) {
                richTextBoxContents.Focus();
                return;
            }
            if (!TogglApi.SendFeedback(
                comboBoxTopic.Text,
                richTextBoxContents.Text,
                openFileDialog.FileName)) {
                return;
            }
            MessageBox.Show("Your feedback was sent successfully.", "Thank you!");
            comboBoxTopic.SelectedIndex = 0;
            richTextBoxContents.Clear();
            openFileDialog.Reset();
            Close();
        }
    }
}
