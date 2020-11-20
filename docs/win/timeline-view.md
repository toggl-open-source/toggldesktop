
# Timeline view

Timeline view is a control which displays time entries in graphical manner as intervals whose placement/length corresponds to time entry start/end time. It also includes tracked activity visualization.

The implementation can be found in `Timeline.xaml`and `Timeline.xaml.cs` in `ui/views` and `TimelineViewModel.cs` in `ui/ViewModels`. We stick to the [RX](http://introtorx.com/Content/v1.0.10621.0/01_WhyRx.html) concept in this whole feature implementation and use [ReactiveUI](https://www.reactiveui.net/docs/handbook/view-models/) library for view models.

"Record activity" is a setting which allows Toggl to track active windows on user's PC and then show this information in the Activity column.

Main view is a grid, which has 3 columns:
1) Time - displays the time in hours. 
2) Time entries - displays time entry blocks whose placement and height corresponds to the time entries start and end time.
3) Activity - displays tracked activity as 15-minutes chunks. It's possible to see the list of activities hovering the mouse over the activity block.

When a day is chosen in the date picker (today by default), the API function `SetViewTimelineDay` is called. Timeline visualization happens as a reaction to API event `toggl_on_timeline` passing the time entries and the timeline data for the selected day.

## Time entry block

Time entries are layed out using `ItemsControl` templated by `Canvas` control. `TimeEntryBlock` class is used as a view model for each item. `TimeEntryBlock.VerticalOffset`, `TimeEntryBlock.HorizontalOffset` and `TimeEntryBlock.Height` properties are used to define time entry block location on `Canvas`. These properties are calculated based on time entry start/end time and current [scale](timeline-view.md#zooming) mode. 

Time entry block is a graphical visualization of a time entry; it includes a rectangle (whose height matches with time entry duration) and all time entry information: description, project name, client, tags. 

The implementation can be found in `TimelineTimeEntryBlock.xaml`and `TimelineTimeEntryBlock.xaml.cs` in `ui/controls` and `TimelineBlockViewModel.cs` in `ui/ViewModels`.

Clicking on time entry block triggers edit popup opening.

Gaps between time entries are filled with buttons, that look like transparent rectangles with dashed borders. Clicking on it will create a new time entry covering the gap between the already existing two.
Implementation can be found in `GapTimeEntryBlock` class (`TimelineBlockViewModel.cs` file in `ui/ViewModels`).

### Running time entry block

Running time entry block is displayed using separate control `TimelineRunningTimeEntryBlock.xaml` and `TimelineRunningTimeEntryBlock.xaml.cs` in `ui/controls`.

### Changing start and end time

User can change start or end time of the time entry using two ways: using the edit view or through resizing. There are `Thumb` controls on the top and bottom of each time entry block, which can be grabbed and dragged vertically. There is an exception for very small blocks (less than `TimelineConstans.MinResizableTimeEntryBlockHeight` in `Resources folder`) - they cannot be resized but only changed though the edit view.

The implementation can be found in `TimelineTimeEntryBlock.xaml`and `TimelineTimeEntryBlock.xaml.cs` in `ui/controls`.

### Create new time entry

A new time entry can be created using click and drag functionality. Start (or end) time and duration of the new time entry will match to the clicked point and drag distance. If user makes just a click or very little drag (less than 2 px) a time entry of one hour long will be created.

The implementation can be found in `Timeline.xaml`and `Timeline.xaml.cs` in `ui/views`.

## Time entry blocks placing algorithm

All the logic for calculating time entry blocks location on `Canvas` is contained in `TimelineViewModel.ConvertTimeEntriesToBlocks`. The main complication is about resolving overlapping time entries, so that minimum number of columns is used. To solve this problem a greedy algorithm described [here](https://www.geeksforgeeks.org/minimum-halls-required-for-class-scheduling/) is used.

## Zooming

Plus and minus signs (located in the top left corner of the Timeline grid) allow to zoom in and out the view. Zooming out reduces the hour height, may also comprise multiple hours into one row for some scale modes.

All implemented zooming modes and their hour heights are listed in dictionary `TimelineConstants.ScaleModes`, `Resources` folder.