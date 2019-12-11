using System;
using System.Linq;
using System.Net.Mail;

namespace TogglDesktop
{
    public static class StringExtensions
    {
        public static int CountSubstrings(this string s, string searchString)
        {
            if (s == null)
                throw new ArgumentNullException(nameof(s));

            if (searchString == null)
                throw new ArgumentNullException(nameof(searchString));

            if (searchString == "")
                throw new ArgumentException("Search string must be non-empty.", nameof(searchString));

            var stringLength = s.Length;
            var subStringLength = searchString.Length;

            var maxFirstIndex = stringLength - subStringLength;

            var count = 1;

            for (int i = 0; i <= maxFirstIndex; i++)
            {
                count++;
                for (int j = 0; j < subStringLength; j++)
                {
                    if (searchString[j] != s[j + i])
                    {
                        count--;
                        break;
                    }
                }
            }

            return count;
        }

        public static bool IsNullOrEmpty(this string s) => string.IsNullOrEmpty(s);

        public static string[] SplitByWhiteSpaceUnlessEnclosedInQuotes(this string str)
        {
            return str.Split('"')
                .Select((element, index) => index % 2 == 0  // If even index
                    ? element.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries)  // Split the item
                    : new[] { element })  // Keep the entire item
                .SelectMany(element => element).ToArray();
        }
        
        public static bool IsValidEmailAddress(this string email)
        {
            if (string.IsNullOrEmpty(email))
            {
                return false;
            }

            try
            {
                var mailAddress = new MailAddress(email);
            }
            catch (FormatException)
            {
                return false;
            }

            return true;
        }
    }
}