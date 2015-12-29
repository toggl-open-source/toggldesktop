
# Edit popup

This file documents the edit popup and edit view of the Windows UI of Toggl Desktop.

The edit popup is a major part of the application since its single view is the primary way of entering and changing details of time entries.

The implementation can be found in `ui/windows/EditViewPopup` and `ui/views/EditView`.


## Edit popup window

The edit view popup window is a small class containing the logic relevant to position itself relative to the main window.

It opens and closes with an animation and can be resized horizontally.


## Edit view

The edit view is a complex control containing input fields for changing all aspects of a time entry.

The view also contains functionality to add clients and projects.

Most of the input fields of the view are linked to auto completion lists provided by the library. These are: the description, project, client, workspace, and tags.

The view is opened and kept up to date with the `OnTimeEntryEditor` event and changes are made to the current time entry with the appropriate API calls.

Care is taken to not override the data in the field a user is editing at the time.

Since the view can be used to add clients and projects as well, it is also responsible for keeping the relevant controls in a valid state at all times, and commit changes and data input by the user whenever the popup is closed.

In general, all changes to the open time entry can be and are committed whenever the user leaves an input field, hits Enter, or closes the popup.