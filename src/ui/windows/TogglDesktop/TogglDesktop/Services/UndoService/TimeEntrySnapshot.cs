using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TogglDesktop.Services.UndoService
{
    public class TimeEntrySnapshot
    {
        public string GUID { get; set; }
        public string Description { get; set; }
        public ulong ProjectId { get; set; }
        public ulong TaskId { get; set; }
        public string Tags { get; set; }
        public bool Billable { get; set; }
        public string Duration { get; set; }
        public ulong Started { get; set; }
        public ulong Ended { get; set; }
    }
}
