
# Overview

This file documents the general structure and architecture of the Windows UI of Toggl Desktop. The focus is to give a quick overview on how the different components of the program are organized and work together.

Most business logic of the application is implemented in C++ inside the TogglDesktopDLL project under `src/lib` ("the library"). All UI code for Toggl Desktop on Windows is contained inside the TogglDesktop project under `src\ui\windows\TogglDesktop\TogglDesktop` ("the ui" / "the interface").


## Architecture

With most business logic contained in the library, Toggl Desktop **loosely** follows the Model-View-Controller (MVC) pattern, where the TogglDesktop project acts as both view and controller.

The different window and view types of the interface correspond to the logical components of the application, and are kept interacting as little as reasonably possible to decrease complexity.

### Windows and views

The central component of the application is the `MainWindow`, which creates most other components, initialises the library, and houses the main views of the application: types implementing the interface `IMainView`. Currently these are the `LoginView` and the `TimeEntryListView`.

The `TimeEntryListView` also contains the `Timer`, which is the application's central interface element.

Another important view is the `EditView`, which is contained in the `EditViewPopup`, which is owned by the `MainWindow`.

### Library-UI interop

To connect library and interface, there exists the static `Toggl` class, which is split into two parts, and files.

`TogglApi.cs` is an auto generated file which contains all constants, structures, methods, and delegates (to define event listener signatures) to interop with the library.

`Toggl.cs` contains a more friendly and better organised public interface exposing the functionality and proper events to the UI.

### Auto completion

Both the `Timer` and the `EditView` make heavy use of auto completion which suggests time entries, descriptions, projects, etc. as the user is typing.

The data for these suggestions is provided by the library through appropriate events. The interface parses this raw data into a more usable object structure and lazily populates the auto completion drop downs when needed.

### WPF bindings

At the moment of writing, (data) bindings are used only sparingly throughout the app. Most views are filled by hand with the data received from the library. There is no particular reason for this choice. While the current implementation performs well, this may be an area of improvement, should bindings prove more efficient or performant than hard-coding ui updates. This has so far not been tested.


## Namespaces and folders

Inside the TogglDesktop project directory, namespaces are contained in nested folders starting with capital letters. Lower-case folders only serve to organize files within a namespace. The base namespace of the project is `TogglDesktop`.

The only files contained in the root folder are application global files, like the library-ui API, and application entry point.

### UI, style sheets

The UI is implemented using WPF, with each control, view, or window consisting of a `*.xaml` and an `*.xaml.cs` file as usual in C#. All these types are contained in the sub-folders of the `ui` directory, mostly sorted by type.

The `ui` folder also contains the application's style sheets in `Resources`, as well as tutorial overlays, OBM experiments and ui elements related to auto completion in the folders and namespaces `Tutorial`, `Experiments`, `AutoCompleteControls` respectively.

The `ui` folder also contains a number of helper types directly related to the interface.

### Images, fonts

Resources like images and fonts are contained in the folder `Resources`. Note that when adding new resources, their build action should be set to "Resource" in most cases, to make sure they are embedded correctly.

### Auto completion, diagnostics, utilities

The `AutoCompletion` folder and namespace contains all logic of the relatively abstract custom auto completion implementation of the application. The sub-directory `Implementation` includes the domain-relevant implementations of the abstract types.

The `Diagnostics` folder and namespace includes types related to measuring and logging performance for debugging purposes.

The folder `utilities` contains several helper types used across the application, including WinAPI interop calls.
