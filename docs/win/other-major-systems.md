

## Application entry point

The application entry point is the `Main` method in `Program.cs`.

The method itself uses a global mutex to detect other running instances of Toggl Desktop. If such an instance is detected, the program tries to activate it, and then terminates.

Otherwise, it initialises the application's error handling and runs the application as specified in `App.xaml`.

This causes the main window to be created, which takes over all other parts of initialisation of Toggl Desktop.


## Library API

Most business logic of Toggl Desktop is located in the library, which is written in C++.

The static class `Toggl`, which is split into `Toggl.cs` and `TogglApi.cs` is responsible for interfacing with the library and creating an easily consumable interface for the rest of the application.

`TogglApi.cs` is auto generated and a number of constants and types to allow for data exchange between library and UI. It also contains a number of delegates, representing the different events the UI can subscribe to, and a number of methods representing API calls accessible to the UI.

Except for the data interchange types, all these members are private. They are wrapped in public methods in `Toggl.cs`. This part of the class is also responsible for subscribing to all library events, and exposing these events again to the UI.


## Styling

The WPF styles of the application are located in a number of files in `ui/Resources` and are merged into the application's resource dictionary in `App.xaml`.

### Custom window styling

To achieve the custom window borders and styling of the Toggl Desktop windows, we use our own window types that override much of the default windows styling. These files can be found in `ui/chrome`.

Care was taken to allow as much of the default windows behaviour - like minimizing and maximising animations, and drop shadows - to be retained, while implementing the application's style according to the design documents.

### Themes

When experimenting with adding an option to allow the user to switch to a more compact layout, we added a theme system that can be used to easily switch out resource dictionaries at runtime.

`ui/ThemeTypes` specifies the different types of themes supported. For each theme, only one resource dictionary can be active at a time.

The default dictionary for a theme type is specified in `App.xaml`, while themes can be switched out with a call to `Activate()` in the static class `ui/Theme.cs`.

Note that resources specified in themes have to be consumed in the UI XAML using dynamic bindings, to ensure they are updated when the themes are switched out.

This system can for example be used for localisation, or color scheme settings.


## Overlays

To allow for features like tutorial or hint overlays, welcome screens, and possibly other things, there is the overlay management system implemented in `ui/Tutorial/`.

The class `TutorialManager.cs` handles the displaying of overlays in the main window, and can display any control that extends `TutorialScreen.cs` as overlay.

It guarantees that there will always be only a single enabled Tutorial screen, and handles fading between different screens.

### Tutorials

There currently is a basic tutorial walking the user through the application's key features: starting, editing and stopping time entries.

The different overlays for the tutorial can be found in `ui/Tutorial` and serve as a good example of how to implement further tutorials and other overlays.

They respond to both user input and API events to advance the tutorial automatically, as the user performs the appropriate actions. They are also able to skip ahead, if the user skips steps in the tutorial process.

### OBM Experiments

To be able to experiment with different options of onboarding users new to Toggl Desktop - and ultimately other things as well - Toggl Desktop is able to run A/B experiments.

The relevant logic is contained inside the class `ExperimentManager.cs` at `ui/Experiments`. The same directory also contains all implemented experiments, which implement the `IExperiment` interface.

Currently only one experiment can be supported at a time, which is set in the `ExperimentManager`.

On startup of the application, the library is informed about which experiment is supported. If the experiment is active, the Toggl backend will assign the user to it and the library will cause the `OnDisplayObmExperiment` event to fire with the appropriate information.