using System;
using System.IO;
using System.Reactive;
using Microsoft.Win32;
using ReactiveUI;
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
        private string _topic;
        private string _feedbackText;
        private string _attachedFileName;
        private bool _isFileSizeError;
        private bool _isFileAttached;
        private bool _isSendingError;
        private bool _isTopicFocused;
        private bool _isFeedbackTextFocused;
        private long _attachedFileSize;

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

        public string Topic
        {
            get => _topic;
            set => this.RaiseAndSetIfChanged(ref _topic, value);
        }

        public string FeedbackText
        {
            get => _feedbackText;
            set => this.RaiseAndSetIfChanged(ref _feedbackText, value);
        }

        public string AttachedFileName
        {
            get => _attachedFileName;
            set => this.RaiseAndSetIfChanged(ref _attachedFileName, value);
        }

        public long AttachedFileSize
        {
            get => _attachedFileSize;
            set => this.RaiseAndSetIfChanged(ref _attachedFileSize, value);
        }

        public bool IsFileAttached
        {
            get => _isFileAttached;
            set => this.RaiseAndSetIfChanged(ref _isFileAttached, value);
        }

        public bool IsSendingError
        {
            get => _isSendingError;
            set => this.RaiseAndSetIfChanged(ref _isSendingError, value);
        }

        public bool IsFileSizeError
        {
            get => _isFileSizeError;
            set => this.RaiseAndSetIfChanged(ref _isFileSizeError, value);
        }

        public bool IsTopicFocused
        {
            get => _isTopicFocused;
            set => this.RaiseAndSetIfChanged(ref _isTopicFocused, value);
        }

        public bool IsFeedbackTextFocused
        {
            get => _isFeedbackTextFocused;
            set => this.RaiseAndSetIfChanged(ref _isFeedbackTextFocused, value);
        }

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