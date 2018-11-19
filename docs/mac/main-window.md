
# Main window

This file documents the main window of the Mac UI of Toggl Desktop.

## Main window views

All views are initialize in the `- (id)initWithWindow:(NSWindow *)window` method of the window.

Views are displayed/hidden by simply adding or removing them as subviews.

There are 3 different views: LoginView, TimeEntryListView and OverlayView;

### Login view

The login view handles both logging in, and signing up for a new Toggl account.

The implementation can be found in `LoginViewController.*`.

In addition, the login view also contains the code to initialise a login using Google OAuth. When Google login is chosen it opens up in-app browser window and allows user to log in to Google.

### Time entry list view

The time entry list view is the central part of the UI. It includes both the timer, and the list of time entries. The view also handles opening and closing a popover with time entry details for editing.

The implementation can be found in `TimeEntryListViewController.*`.


#### Timer

The timer provides the functionality to start and stop tracking.

Its implementation can be found in `TimerEditViewControler.*`.

When stopped (not tracking) the timer allows the user to input descriptions, as well as select projects through an auto completing text box. It also allows to start the timer with the given info using the start button.

When running, the timer displays the time entries information, including the current running time. The user can stop tracking using the stop button, or open the edit popup to edit the running time entry, by clicking on the displayed information in the running timer.

#### Time entry list

The time entry list contains the list of past time entries.

Its implementation can be found in `NSUnstripedTableView.*` inside `TimeEntryListViewController`.

The `NSUnstripedTableView` handles entry deletion, keyboard shortcuts and getting selected item in the list. All the list loading and editing popup triggering is done in `TimeEntryListViewController`.