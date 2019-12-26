using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Globalization;

namespace TogglDesktop.AutoCompletion
{
    class ListBoxItem
    {
        public string Text { get; set; }
        public string ProjectLabel { get; set; }
        public string ProjectColor { get; set; }
        public string Description { get; set; }
        public string ProjectAndTaskLabel { get; set; }
        public string TaskLabel { get; set; }
        public string ClientLabel { get; set; }
        public string Category { get; set; }
        private string _workspaceName = "";
        public string WorkspaceName
        {
            get
            {
                return this._workspaceName;
            }
            set
            {
                this._workspaceName = CultureInfo.CurrentCulture.TextInfo.ToTitleCase(value);
            }
        }
        public int Type { get; set; }
        public int Index { get; set; }
    }
}
