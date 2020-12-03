using System;
using System.Collections.Generic;

namespace TogglDesktop.Resources
{
    public static class TimelineConstants
    {
        public const int MaxActivityBlockDurationInSec = 900;
        public const double MinTimeEntryBlockHeight = 2;
        public const double MinResizableTimeEntryBlockHeight = 14; //Resize handles take 5 px each (10 in total)
        public const double MinShowTEDescriptionHeight = 20;
        public const double DescriptionOpacity = 0.2;
        public const double TimeEntryBlockWidth = 20;
        public const double GapBetweenOverlappingTEs = 5;
        public const double AcceptableBlocksOverlap = 1e-5;
        public const double MinGapTimeEntryHeight = 10;
        public const int DefaultTimeEntryLengthInSeconds= (int) (DefaultTimeEntryLengthInHours * 3600);
        public const double DefaultTimeEntryLengthInHours = 1;

        public static IReadOnlyDictionary<int, int> ScaleModes { get; } = new Dictionary<int, int>()
        {
            {0, 200},
            {1, 100},
            {2, 50},
            {3, 25}
        };
    }
}
