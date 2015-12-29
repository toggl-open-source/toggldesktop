
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
    - [Edit popup](edit-popup.md)
        - [Edit view](edit-popup.md#edit-view)
- Other major systems
    - Application entry point
    - API to library
    - Styling
        - Custom window styling
        - Themes
    - Overlays
        - Tutorials
        - OBM Experiments
    - [Autocompletion](autocompletion.md)