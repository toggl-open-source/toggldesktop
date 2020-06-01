using System.Linq;

namespace TogglDesktop.ViewModels
{
    public static class PasswordEx
    {
        public static bool IsEightCharactersOrMore(this string s) => s.Length >= 8;
        public static bool IsLowercaseAndUppercase(this string s) => s.Any(char.IsUpper) && s.Any(char.IsLower);
        public static bool IsAtLeastOneNumber(this string s) => s.Any(char.IsDigit);

        public static bool AllRulesSatisfied(this string s) =>
            IsEightCharactersOrMore(s) && IsLowercaseAndUppercase(s) && IsAtLeastOneNumber(s);
    }
}