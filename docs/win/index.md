
# Toggl Desktop Windows UI Documentation

These files document the general structure and architecture of the Windows UI of Toggl Desktop and its main components.

The total application is composed of two major parts: The interface, written in C#, using WPF, and the library containing most business logic. This documentation deals only with the interface, and the API as used by it.

## Table of contents

- [Overview](overview.md)
    - [Architecture](overview.md#architecture)
    - [Namespaces and folders](overview.md#namespaces-and-folders)
- Main UI parts
    - [Main window](main-window.md)
        - [Login view](main-window.md#login-view)
        - [Time entry list view](main-window.md#time-entry-list-view)
            - [Timer](main-window.md#timer)
            - [Time entry list](main-window.md#time-entry-list)
		- [Timeline view](timeline-view.md)
			- [Time entry block](timeline-view.md#time-entry-block)
				- [Running time entry block](timeline-view.md#running-time-entry-block)
				- [Changing start and end time](timeline-view.md#changing-start-and-end-time)
				- [Create new time entry](timeline-view.md#create-new-time-entry)
			- [Time entry blocks placing algorithm](timeline-view.md#time-entry-blocks-placing-algorithm)
			- [Zooming](timeline-view.md#zooming)
    - [Edit popup](edit-popup.md)
        - [Edit view](edit-popup.md#edit-view)
- [Other major systems](other-major-systems.md)
    - [Application entry point](other-major-systems.md#application-entry-point)
    - [Library API](other-major-systems.md#library-api)
    - [Styling](other-major-systems.md#styling)
        - [Custom window styling](other-major-systems.md#custom-window-styling)
        - [Themes](other-major-systems.md#themes)
    - [Overlays](other-major-systems.md#overlays)
        - [Tutorials](other-major-systems.md#tutorials)
        - [OBM Experiments](other-major-systems.md#obm-experiments)
    - [Autocompletion](autocompletion.md)