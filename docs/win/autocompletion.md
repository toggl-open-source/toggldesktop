
# Autocompletion

To handle the context specific requirements of inputting data into Toggl, the Windows UI of Toggl Desktop includes an entirely custom system to handle autocompletion and suggestions of user input.

This system is used all across the application's edit view as well as in the timer, to help the user quickly complete their input.

The abstract implementation of the system can be found in the `AutoCompletion` namespace, while the Toggl specific implementations of these abstract types can be found in `AutoCompletion.Implementation`.

The UI controls displayed inside the autocompletion popups can be found in `ui/AutoCompleteControls`, while the popup itself is implemented in `ui/controls/AutoCompletionPopup`.

The logic to take data given by the library and build the autocompletion data structures is in `AutoCompletion/AutoCompleteControllers.cs`.


## Overview

A working autocompletion/suggestion system requires the use of an `AutoCompleteController`, which contains a tree structure corresponding to the categories and items that can be shown to the user. The different kind of items and categories can be found in the `AutoCompletion` namespace.

Each item and category are responsible to match themselves against given input strings and set their own visibility appropriately.

The concrete types found in `AutoCompletion.Implementation` are responsible for correctly passing the search string down to the abstract base class, and to generate the correct UI control for the item or category.

To significantly improve the performance of the application, all autocompletion controls are recycled in a static object pool. This allows reusing the same controls when discarding and rebuilding the autocompletion list.

The `AutoCompletionPopup`, which represents the UI container for the autocompletion list and contains the logic to respond to user controls and navigation, is also responsible for this recycling.

Since the autocompletion data can update rather often, the list is filled lazily when it is needed.