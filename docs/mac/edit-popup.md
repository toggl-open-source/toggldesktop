
# Edit popup

This file documents the edit popup and edit view of the Mac UI of Toggl Desktop.

The edit popup is a major part of the application since its single view is the primary way of entering and changing details of time entries.

The implementation can be found in `TimeEntryEditViewController.*`.


## Edit popup window

The edit view popup window is a popover connected to the TimeEntryList. It is so because when connecting to certain time entry inside the list, the connection would be broken when list refreshes while edit popup is opened. 

It opens and closes with an animation and can be resized horizontally.


## Edit view

The edit view is a complex control containing input fields for changing all aspects of a time entry.

The view also contains functionality to add clients and projects.

Most of the input fields of the view are linked to auto completion lists provided by the library. These are: the description, project, client, workspace, and tags.

Description and project autocomplete is a custom component with grouping and colorful texts. Other items use regular built in comboboxes with datasources that work as autocompletes.

Care is taken to not override the data in the field a user is editing at the time.

All data is saved as user moves away from the field. The exception is adding new project. If new project is created it will be created and attached to the entry when the popup closes.