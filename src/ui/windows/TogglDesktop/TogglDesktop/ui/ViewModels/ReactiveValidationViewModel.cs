using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Concurrency;
using System.Reactive.Linq;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using ReactiveUI.Validation.Abstractions;
using ReactiveUI.Validation.Components.Abstractions;
using ReactiveUI.Validation.Contexts;

namespace TogglDesktop.ViewModels
{
    public class ReactiveValidationViewModel<TViewModel> : ReactiveObject, INotifyDataErrorInfo, IValidatableViewModel
    {
        [Reactive]
        private bool IsActive { get; set; }

        public ReactiveValidationViewModel(bool isActive = true, IScheduler scheduler = null)
        {
            ValidationContext = new ValidationContext(scheduler);
            IsActive = isActive;

            ValidationContext.ValidationStatusChange
                .Subscribe(args => OnErrorsChanged(string.Empty));
            ValidationContext.Valid.Select(valid => !valid).Subscribe(v => HasErrors = v);
            this.WhenAnyValue(x => x.IsActive).Subscribe(_ =>
                _propertyMemberNameDictionary.Keys.ForEach(OnErrorsChanged));
        }

        protected void OnErrorsChanged(string propertyName)
        {
            ErrorsChanged?.Invoke(this, new DataErrorsChangedEventArgs(propertyName));
        }

        /// <inheritdoc />
        public event EventHandler<DataErrorsChangedEventArgs> ErrorsChanged;

        public IEnumerable GetErrors(string propertyName)
        {
            var fullPropertyName = GetMemberInfoName(propertyName);
            if (!IsActive || propertyName.IsNullOrEmpty()) return Enumerable.Empty<string>();

            var temp = ValidationContext.Validations
                .OfType<IPropertyValidationComponent<TViewModel>>()
                .Where(v => !v.IsValid && v.ContainsPropertyName(fullPropertyName))
                .SelectMany(v => v.Text)
                .ToArray();
            return temp;
        }

        private readonly Dictionary<string, string> _propertyMemberNameDictionary = new Dictionary<string, string>();
        private string GetMemberInfoName(string propertyName)
        {
            if (propertyName == null) return "";

            if (!_propertyMemberNameDictionary.ContainsKey(propertyName))
            {
                _propertyMemberNameDictionary.Add(propertyName, GetMemberInfoName());
            }

            return _propertyMemberNameDictionary[propertyName];

            string GetMemberInfoName() => GetType()
                .GetMember(propertyName)
                .FirstOrDefault()?
                .ToString();
        }

        /// <inheritdoc />
        [Reactive]
        public bool HasErrors { get; set; }

        public void ActivateValidation() => IsActive = true;

        public void SnoozeValidation() => IsActive = false;

        public ValidationContext ValidationContext { get; }
    }
}
