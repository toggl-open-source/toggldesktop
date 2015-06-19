using System;
using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class StringAutoCompleteController : AutoCompleteController
    {
        private StringAutoCompleteController(List<IAutoCompleteListItem> list)
            : base(list)
        {
        }

        public static AutoCompleteController From(IEnumerable<string> items)
        {
            var list = items.Select(i => new StringAutoCompleteItem(i)).Cast<IAutoCompleteListItem>().ToList();

            return new StringAutoCompleteController(list);
        }
    }
}