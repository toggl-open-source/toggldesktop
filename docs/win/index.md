
# Toggl Desktop Windows UI Documentation

These files document the general structure and architecture of the Windows UI of Toggl Desktop and its main components.

The total application is composed of two major parts: The interface, written in C#, using WPF, and the library containing most business logic. This documentation deals only with the interface, and the API as used by it.

## Table of contents

- [Overview](overview.md)
    - [Namespaces and folders](overview.md#namespaces-and-folders)
    - [Architecture](overview.md#architecture)
- Main UI parts
    - Main window
        - Login view
        - Time entry list view
            - Time entry cell
    - Edit popup
        - Edit view
    - Other windows
        - Preferences
        - About window
        - Feedback window
        - Idle notification
        - Autotracker notification
        - Message boxes
- Other major systems
    - Application entry point
    - API to library
    - Styling
        - Custom window styling
        - Themes
    - Overlays
        - Tutorials
        - OBM Experiments
    - Autocompletion