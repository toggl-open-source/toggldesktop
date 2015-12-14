
# Toggl Desktop Windows UI Documentation

These files document the general structure and architecture of the Windows UI of Toggl Desktop.

The total application is composed of two major parts: The interface, written in C#, using WPF, and the library containing most business logic. This documentation deals only with the interface, and the API as used by it.

## Table of contents

- Namespace and folder overview
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