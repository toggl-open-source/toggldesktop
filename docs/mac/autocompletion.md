
# Autocompletion

To handle the context specific requirements of inputting data into Toggl, the Windows UI of Toggl Desktop includes an entirely custom system to handle autocompletion and suggestions of user input.

This system is used all across the application's edit view as well as in the timer, to help the user quickly complete their input.

Autocomplete system consists of the following components:

## AutoCompleteInput

This is the item element. It tracks keyboard events and shows/hides autocomplete suggestions dropdown table.

## AutoCompleteTable

Keeps track of the currently highlighted / selected item. Handles mouse events on table items.

## AutoCompleteTableCell

Handles data display on cell view item. All font colors sizes an styles are attached here.

## LiteAutoCompleteDataSoure

This is where all the data is handled:
- Filling in data to the table
- Creating categories and groups
- Filtering table items based on the typed in text

## AutoComopleteTableContainer

Simple Table container for the AutoCompleteTable view.
