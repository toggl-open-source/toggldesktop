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
    public partial class PreferencesWindowController : TogglForm
    {
        public PreferencesWindowController()
        {
            InitializeComponent();

            Toggl.OnSettings += OnSettings;
            Toggl.OnLogin += OnLogin;
        }

        private void PreferencesWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            btnRecordShowHideShortcut.Text = kRecordShortcut;
            btnRecordShowHideShortcut.Tag = null;

            btnRecordStartStopShortcut.Text = kRecordShortcut;
            btnRecordStartStopShortcut.Tag = null;

            btnClearShowHideShortcut.Tag = null;
            btnClearStartStopTimer.Tag = null;

            e.Cancel = true;
            Hide();
        }

        void OnLogin(bool open, UInt64 user_id)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnLogin(open, user_id); });
                return;
            }
            checkBoxRecordTimeline.Enabled = !open && user_id != 0;
            checkBoxRecordTimeline.Checked = Toggl.IsTimelineRecordingEnabled();
        }

        void OnSettings(bool open, Toggl.Settings settings)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnSettings(open, settings); });
                return;
            }
            groupBoxProxySettings.Enabled = settings.UseProxy;
            checkBoxUseProxy.Checked = settings.UseProxy;
            textBoxProxyHost.Text = settings.ProxyHost;
            textBoxProxyPort.Text = settings.ProxyPort.ToString();
            textBoxProxyUsername.Text = settings.ProxyUsername;
            textBoxProxyPassword.Text = settings.ProxyPassword;
            checkBoxIdleDetection.Checked = settings.UseIdleDetection;
            checkBoxRecordTimeline.Checked = settings.RecordTimeline;
            checkBoxOnTop.Checked = settings.OnTop;
            checkBoxRemindToTrackTime.Checked = settings.Reminder;
            textBoxIdleMinutes.Text = settings.IdleMinutes.ToString();

            // Load shortcuts
            {
                string keyCode = Properties.Settings.Default.ShowKey;
                if (keyCode != "" && keyCode != null)
                {
                    TogglDesktop.ModifierKeys modifiers =
                        Properties.Settings.Default.ShowModifiers;
                    btnRecordShowHideShortcut.Text = keyEventToString(modifiers, keyCode);
                }
            }

            {
                string keyCode = Properties.Settings.Default.StartKey;
                if (keyCode != "" && keyCode != null)
                {
                    TogglDesktop.ModifierKeys modifiers =
                        Properties.Settings.Default.StartModifiers;
                    btnRecordStartStopShortcut.Text = keyEventToString(modifiers, keyCode);
                }
            }

            if (open)
            {
                Show();
                TopMost = true;
            }
        }

        private void checkBoxUseProxy_CheckedChanged(object sender, EventArgs e)
        {
            groupBoxProxySettings.Enabled = checkBoxUseProxy.Checked;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            ulong port = 0;
            if (!ulong.TryParse(textBoxProxyPort.Text, out port))
            {
                port = 0;
            }

            ulong idleMinutes = 5;
            if (!ulong.TryParse(textBoxIdleMinutes.Text, out idleMinutes))
            {
                idleMinutes = 5;
            }

            Toggl.Settings settings = new Toggl.Settings();

            settings.UseProxy = checkBoxUseProxy.Checked;
            settings.ProxyPort = port;
            settings.ProxyHost = textBoxProxyHost.Text;
            settings.ProxyUsername = textBoxProxyUsername.Text;
            settings.ProxyPassword = textBoxProxyPassword.Text;

            settings.UseIdleDetection = checkBoxIdleDetection.Checked;
            settings.OnTop = checkBoxOnTop.Checked;
            settings.Reminder = checkBoxRemindToTrackTime.Checked;
            settings.IdleMinutes = idleMinutes;

            settings.MenubarTimer = true;
            settings.DockIcon = true;

            settings.RecordTimeline = checkBoxRecordTimeline.Checked;

            if (!Toggl.SetSettings(settings))
            {
                return;
            }

            // Save shortcut keys

            if (btnRecordShowHideShortcut.Tag != null)
            {
                Utils.SetShortcutForShow((KeyEventArgs)btnRecordShowHideShortcut.Tag);
            }
            if (btnClearShowHideShortcut.Tag != null)
            {
                Utils.SetShortcutForShow(null);
            }

            if (btnRecordStartStopShortcut.Tag != null)
            {
                Utils.SetShortcutForStart((KeyEventArgs)btnRecordStartStopShortcut.Tag);
            }
            if (btnClearStartStopTimer.Tag != null)
            {
                Utils.SetShortcutForStart(null);
            }

            Close();
        }

        private void checkBoxIdleDetection_CheckedChanged(object sender, EventArgs e)
        {
            textBoxIdleMinutes.Enabled = checkBoxIdleDetection.Checked;
        }

        private void btnRecordShowHideShortcut_Click(object sender, EventArgs e)
        {
            btnRecordShowHideShortcut.Text = kTypeShortcut;
        }

        const string kTypeShortcut = "Type Shortcut";

        private void btnRecordStartStopShortcut_Click(object sender, EventArgs e)
        {
            btnRecordStartStopShortcut.Text = kTypeShortcut;
        }

        string kRecordShortcut = "Record Shortcut";

        private void btnClearShowHideShortcut_Click(object sender, EventArgs e)
        {
            btnRecordShowHideShortcut.Text = kRecordShortcut;
            btnClearShowHideShortcut.Tag = true;
        }

        private void btnClearStartStopTimer_Click(object sender, EventArgs e)
        {
            btnRecordStartStopShortcut.Text = kRecordShortcut;
            btnClearStartStopTimer.Tag = true;
        }

        string keyEventToString(TogglDesktop.ModifierKeys modifiers, string keyCode)
        {
            string res = "";
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Alt))
            {
                res += "Alt + ";
            }
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Control))
            {
                res += "Ctrl + ";
            }
            if (modifiers.HasFlag(TogglDesktop.ModifierKeys.Shift))
            {
                res += "Shift + ";
            }
            res += keyCode;
            return res;
        }

        void handleKeyPress(Button btn, Button clearBtn, KeyEventArgs e)
        {
            if (kTypeShortcut != btn.Text)
            {
                return;
            }
            if (!e.Alt && !e.Control)
            {
                return;
            }
            btn.Tag = e;
            btn.Text = keyEventToString(Utils.GetModifiers(e), Utils.GetKeyCode(e));
            clearBtn.Tag = null;
        }

        private void btnRecordShowHideShortcut_KeyUp(object sender, KeyEventArgs e)
        {
            handleKeyPress(btnRecordShowHideShortcut, btnClearShowHideShortcut, e);
        }

        private void btnRecordStartStopShortcut_KeyUp(object sender, KeyEventArgs e)
        {
            handleKeyPress(btnRecordStartStopShortcut, btnClearStartStopTimer, e);
        }
    }
}
