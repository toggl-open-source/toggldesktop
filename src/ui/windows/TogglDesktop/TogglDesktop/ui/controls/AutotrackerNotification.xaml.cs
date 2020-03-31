﻿using System;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class AutotrackerNotification
    {
        private ulong projectId;
        private ulong taskId;

        public AutotrackerNotification(TaskbarIcon icon, MainWindow mainWindow)
            : base(icon, mainWindow)
        {
            this.InitializeComponent();
            Toggl.OnAutotrackerNotification.ObserveOnDispatcher().Subscribe(this.onAutotrackerNotification);
        }

        private void onAutotrackerNotification((string projectName, ulong projectId, ulong taskId) x)
        {
            var (projectName, projectId, taskId) = x;
            this.Message = @$"Start tracking ""{projectName}""?";
            this.projectId = projectId;
            this.taskId = taskId;

            this.RemoveFromParent();

            _icon.ShowNotification(this, PopupAnimation.Slide, TimeSpan.FromSeconds(6));
        }

        private void onStartButtonClick(object sender, RoutedEventArgs e)
        {
            Close();
            Toggl.Start("", "", this.taskId, this.projectId, null, null);
            _parentWindow.ShowOnTop();
        }
    }
}
