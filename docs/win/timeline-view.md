
# Timeline view

Timeline view is a control which displays time entries in graphical manner as intervals whose placement and length correponds to time entry start and end times. It also includes tracked activity visualization.

The implementation can be found in `Timeline.xaml`and `Timeline.xaml.cs` in `ui/views` and `TimelineViewModel.cs` in `ui/ViewModels`.

Turning Record activity is a setting which allows Toggl to track active windows on user's PC and then show this information in the Activity column.

Main view is a grid, which has 3 columns:
1) Time - displays the time in hours. 
2) Time entries - displays time entry block whose placement and height corresponds to the time entry start and end time.
3) Activity - displays tracked activity as 15 minutes chunks. It's possible to see the list of activities hanging the mouse over the activity block.

## Time entry block

Time entry block corresponds to each time entry, includes a rectangle (whose height corresponds to time entry duration) and all time entry information: description, project name, client, tags. 

The implementation can be found in `TimelineTimeEntryBlock.xaml`and `TimelineTimeEntryBlock.xaml.cs` in `ui/controls` and `TimelineBlockViewModel.cs` in `ui/ViewModels`.

## Running time entry block

Running time entry block is displayed using separate `TimelineRunningTimeEntryBlock.xaml` and `TimelineRunningTimeEntryBlock.xaml.cs` in `ui/controls`.