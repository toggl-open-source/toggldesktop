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
    public partial class FeedbackWindowController : TogglForm
    {
        public FeedbackWindowController()
        {
            InitializeComponent();
        }

        private void FeedbackWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();
            openFileDialog.Reset();
            fileNameLabel.Text = "";
            e.Cancel = true;
        }

        private void FeedbackWindowController_Load(object sender, EventArgs e)
        {
            comboBoxTopic.SelectedIndex = 0;
        }

        private void buttonUploadImage_Click(object sender, EventArgs e)
        {
            if(openFileDialog.ShowDialog() == DialogResult.OK)
            {
                fileNameLabel.Text = "Uploaded: " + openFileDialog.SafeFileName;
            }
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            if (comboBoxTopic.SelectedIndex == 0)
            {
                topicreminder.Visible = true;
                comboBoxTopic.Focus();
                return;
            }

            if (richTextBoxContents.TextLength == 0) {
                richTextBoxContents.Focus();
                return;
            }
            if (!Toggl.SendFeedback(
                comboBoxTopic.Text,
                richTextBoxContents.Text,
                openFileDialog.FileName)) {
                return;
            }
            try
            {
                MainWindowController.DisableTop();
                MessageBox.Show("Your feedback was sent successfully.", "Thank you!");
            } finally {
                MainWindowController.EnableTop();
            }
            comboBoxTopic.SelectedIndex = 0;
            richTextBoxContents.Clear();
            Close();
        }

        private void comboBoxTopic_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (comboBoxTopic.SelectedIndex != 0)
            {
                topicreminder.Visible = false;
            }
        }
    }
}
