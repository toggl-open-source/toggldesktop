using System;
using System.IO;
using System.Reactive;
using Microsoft.Win32;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using ReactiveUI.Validation.Extensions;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class FeedbackWindowViewModel : ReactiveValidationObject<FeedbackWindowViewModel>
    {
        private readonly Action _refreshValidationBindings;
        private readonly Action _onFeedbackSent;
        private readonly ValidationHelper _topicValidation;
        private readonly ValidationHelper _feedbackTextValidation;

        public FeedbackWindowViewModel(Action refreshValidationBindings, Action onFeedbackSent)
        {
            _refreshValidationBindings = refreshValidationBindings;
            _onFeedbackSent = onFeedbackSent;
            SendCommand = ReactiveCommand.Create(Send);
            AttachFileCommand = ReactiveCommand.Create(AttachFile);
            RemoveFileCommand = ReactiveCommand.Create(RemoveFile);

            _topicValidation = this.ValidationRule(
                x => x.Topic,
                topic => !string.IsNullOrEmpty(topic),
                "Please choose a topic");
            _feedbackTextValidation = this.ValidationRule(
                x => x.FeedbackText,
                feedbackText => !string.IsNullOrEmpty(feedbackText),
                "Please add your feedback");
        }

        public string[] PossibleTopics { get; } = {"Report bug", "Suggest a feature", "Other"};

        [Reactive]
        public string Topic { get; set; }

        [Reactive]
        public string FeedbackText { get; set; }

        [Reactive]
        public string AttachedFileName { get; private set; }

        [Reactive]
        public long AttachedFileSize { get; private set; }

        [Reactive]
        public bool IsFileAttached { get; private set; }

        [Reactive]
        public bool IsSendingError { get; private set; }

        [Reactive]
        public bool IsFileSizeError { get; private set; }

        [Reactive]
        public bool IsTopicFocused { get; set; }

        [Reactive]
        public bool IsFeedbackTextFocused { get; set; }

        public ReactiveCommand<Unit, Unit> SendCommand { get; }

        public ReactiveCommand<Unit, Unit> AttachFileCommand { get; }

        public ReactiveCommand<Unit, Unit> RemoveFileCommand { get; }

        private void Send()
        {
            IsFileSizeError = false;
            IsSendingError = false;
            IsTopicFocused = false;
            IsFeedbackTextFocused = false;

            _refreshValidationBindings();

            if (this.HasErrors)
            {
                if (!_topicValidation.IsValid)
                {
                    IsTopicFocused = true;
                }
                else if (!_feedbackTextValidation.IsValid)
                {
                    IsFeedbackTextFocused = true;
                }

                return;
            }

            if (Toggl.SendFeedback(Topic, FeedbackText, AttachedFileName))
            {
                _onFeedbackSent();
            }
            else
            {
                IsSendingError = true;
            }
        }

        private void AttachFile()
        {
            IsFileSizeError = false;
            var dialog = new OpenFileDialog();
            if (dialog.ShowDialog() == true)
            {
                var fileInfo = new FileInfo(dialog.FileName);
                var sizeInBytes = fileInfo.Length;
                if (sizeInBytes > 5 * 1024 * 1024)
                {
                    IsFileAttached = false;
                    IsFileSizeError = true;
                }
                else
                {
                    // success
                    IsFileAttached = true;
                    AttachedFileName = dialog.FileName;
                    AttachedFileSize = sizeInBytes;
                }
            }
        }

        private void RemoveFile()
        {
            IsFileAttached = false;
            AttachedFileName = default;
            AttachedFileSize = default;
            IsSendingError = false;
        }
    }
}