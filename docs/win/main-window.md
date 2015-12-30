
# Main window

This file documents the main window of the Windows UI of Toggl Desktop.

The main window is the central part of the UI, host to the application's main views, and serves as a parent of most other parts of the UI.

The implementation of the main window can be found in `MainWindow.xaml` and `MainWindow.xaml.cs` in `ui/windows`.

The main window's constructor creates the rest of the UI and afterwards initialises the library.

It is also responsible for handling keyboard shortcuts and context menu actions, and provides all shutdown logic.

## Main window views

Main views are UI controls implementing the `IMainView` interface, should be children of the main window's grid, and should be included in the array of main views.

At any given time, one main view can be active in the main window. This constraint is preserved by the main window itself.

Currently, there are two main views, the login view and the time entry list view.

### Login view

The login view handles both logging in, and signing up for a new Toggl account.

The implementation can be found in `LoginView.xaml` and `LoginView.xaml.cs` in `ui/views`.

As such it is a state machine with these two states. While trying to log in or sign up, the views interface is disabled, while the operation is executed asynchronously.

In addition, the login view also contains the code to initialise a login using Google OAuth.

### Time entry list view

The time entry list view is the central part of the UI. It includes both the timer, and the list of time entries.

The implementation can be found in `TimeEntryCellList.xaml`and `TimeEntryCellList.xaml.cs` in `ui/views`.

The view handles the routing of some events and calls to the two components, and is responsible for filling the time entry list with entry cells.

#### Timer

The timer provides the functionality to start and stop tracking.

Its implementation can be found in `Timer.xaml` and `Timer.xaml.cs` in `ui/views`.

When stopped (not tracking) the timer allows the user to input descriptions, as well as select projects through an auto completing text box. It also allows the user to input a time, as well as start the timer with the given info using the start button.

When running, the timer displays the time entries information, including the current running time. The user can stop tracking using the stop button, or open the edit popup to edit the running time entry, but clicking on the displayed information.

#### Time entry list

The time entry list contains the list of past time entries.

Its implementation can be found in `TimeEntryList.xaml` and `TimeEntryList.xaml.cs` in `ui\controls`.

The class itself is responsible for handling a greyed out overlay that obscures other time entries, if one is selected for editing, and for handling the highlight for keyboard navigation, which consists of an imposter time entry that renders on top of the overlay to guarantee visibility.

Next to holding the list of time entries, the class' main purpose is to ensure these highlights are always in a correct and valid state.