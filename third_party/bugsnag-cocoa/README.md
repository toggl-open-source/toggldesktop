Bugsnag Notifier for Cocoa
==========================

The Bugsnag Notifier for Cocoa gives you instant notification of exceptions thrown from your *iOS* or *OSX* applications.
The notifier hooks into `NSSetUncaughtExceptionHandler`, which means any uncaught exceptions will trigger a notification to be sent to your Bugsnag dashboard. Bugsnag will also monitor for fatal signals sent to your application, for example, Segmentation Faults.

[Bugsnag](https://bugsnag.com) captures errors in real-time from your web, mobile and desktop applications, helping you to understand and resolve them as fast as possible. [Create a free account](https://bugsnag.com) to start capturing exceptions from your applications.


Installation & Setup
--------------------

###Using CocoaPods (Recommended)

[Cocoapods](http://cocoapods.org/) is a library management system for iOS/OSX which allows you to manage your libraries, detail your dependencies and handle updates nicely. It is the recommended way of installing the Bugsnag Cocoa library.

-   Add Bugsnag to your `Podfile`

    ```ruby
    pod 'Bugsnag'
    ```

-   Install Bugsnag

    ```bash
    pod install
    ```

-   Import the `Bugsnag.h` file into your application delegate.

    ```objective-c
    #import "Bugsnag.h"
    ```

-   In your `application:didFinishLaunchingWithOptions:` method, initialize Bugsnag by calling,

    ```objective-c
    [Bugsnag startBugsnagWithApiKey:@"your-api-key-goes-here"];
    ```

###Without Cocoapods

-   Clone the github bugsnag repository.

    ```shell
    git clone git@github.com:bugsnag/bugsnag-cocoa.git
    ```

-   Copy the bugsnag directory from inside the bugsnag-cocoa repository into your project.

-   Delete any platform you are not using. For example, if you are writing iOS code, delete the OSX directory from within the bugsnag directory.

-   Import the `Bugsnag.h` file into your application delegate.

    ```objective-c
    #import "Bugsnag.h"
    ```

-   In your `application:didFinishLaunchingWithOptions:` method, register with bugsnag by calling,

    ```objective-c
    [Bugsnag startBugsnagWithApiKey:@"your-api-key-goes-here"];
    ```

-   Add the SystemConfiguration Framework

    Click your project, click on your main target, then navigate to "Build Phases", select "Link Binary with Libraries" and click on the "+" button. Then add SystemConfiguration.framework.

-   If you are building an OSX project, you also need to link with the ExceptionHandling Framework. This step is not required for iOS.

    Click your project, click on your main target, then navigate to "Build Phases", select "Link Binary with Libraries" and click on the "+" button. Then add ExceptionHandling.framework.

-   Add a build phase to upload the symbolication information to Bugsnag

    From the same "Build Phases" screen, click the plus in the bottom right of the screen labelled "Add Build Phase", then select "Add Run Script". Then expand the newly added "Run Script" section, and set the shell to `/usr/bin/ruby` and copy the following script into the text box,

    ```ruby
    if ENV["DEBUG_INFORMATION_FORMAT"] != "dwarf-with-dsym"
      exit
    end

    fork do
      Process.setsid
      STDIN.reopen("/dev/null")
      STDOUT.reopen("/dev/null", "a")
      STDERR.reopen("/dev/null", "a")

      require 'shellwords'

      Dir["#{ENV["DWARF_DSYM_FOLDER_PATH"]}/#{ENV["DWARF_DSYM_FILE_NAME"]}/Contents/Resources/DWARF/*"].each do |dsym|
        system("curl -F dsym=@#{Shellwords.escape(dsym)} -F projectRoot=#{Shellwords.escape(ENV["PROJECT_DIR"])} https://upload.bugsnag.com/")
      end
    end
    ```

Send Non-Fatal Exceptions to Bugsnag
------------------------------------

If you would like to send non-fatal exceptions to Bugsnag, you can pass any `NSException` to the `notify` method:

```objective-c
[Bugsnag notify:[NSException exceptionWithName:@"ExceptionName" reason:@"Something bad happened" userInfo:nil]];
```

You can also send additional meta-data with your exception:

```objective-c
[Bugsnag notify:[NSException exceptionWithName:@"ExceptionName" reason:@"Something bad happened" userInfo:nil]
       withData:[NSDictionary dictionaryWithObjectsAndKeys:@"username", @"bob-hoskins", nil]];
```

Adding Tabs to Bugsnag Error Reports
------------------------------------

If you want to add a tab to your Bugsnag error report, you can call the `addToTab` method:

```objective-c
[Bugsnag addAttribute:@"username" withValue:@"bob-hoskins" toTabWithName:@"user"];
[Bugsnag addAttribute:@"registered-user" withValue:@"yes" toTabWithName:@"user"];
```

This will add a user tab to any error report sent to bugsnag.com that contains the username and whether the user was registered or not.

You can clear a single attribute on a tab by calling:

```objective-c
[Bugsnag addAttribute:@"username" withValue:nil toTabWithName:@"user"];
```

or you can clear the entire tab:

```objective-c
[Bugsnag clearTabWithName:@"user"];
```

Configuration
-------------

###context

Bugsnag uses the concept of "contexts" to help display and group your errors. Contexts represent what was happening in your application at the time an error occurs. The Notifier will set this to be the top most UIViewController, but if in a certain case you need to override the context, you can do so using this property:

```objective-c
[Bugsnag configuration].context = @"MyUIViewController";
```

###userId

Bugsnag helps you understand how many of your users are affected by each error. In order to do this, we send along a userId with every exception. By default we will generate a unique ID and send this ID along with every exception from an individual device.

If you would like to override this `userId`, for example to set it to be a username of your currently logged in user, you can set the `userId` property:

```objective-c
[Bugsnag configuration].userId = @"leeroy-jenkins";
```

###releaseStage

In order to distinguish between errors that occur in different stages of the application release process a release stage is sent to Bugsnag when an error occurs. This is automatically configured by the notifier to be "production", unless DEBUG is defined during compilation. In this case it will be set to "development". If you wish to override this, you can do so by setting the releaseStage property manually:

```objective-c
[Bugsnag configuration].releaseStage = @"development";
```

###notifyReleaseStages

By default, we notify Bugsnag of all exceptions that happen in your app. If you would like to change which release stages notify Bugsnag of exceptions you can set the `notifyReleaseStages` property:

```objective-c
[Bugsnag configuration].notifyReleaseStages = [NSArray arrayWithObjects:@"production", nil];
```

###autoNotify

By default, we will automatically notify Bugsnag of any fatal exceptions in your application. If you want to stop this from happening, you can set `autoNotify` to NO:

```objective-c
[Bugsnag configuration].autoNotify = NO;
```

###enableSSL

By default, Bugsnag enables the use of SSL encryption when sending errors to Bugsnag. If you want to use an unencrypted connection to Bugsnag, you can set `enableSSL` to NO:

```objective-c
[Bugsnag configuration].enableSSL = NO;
```

ARC Support
-----------

Bugsnag uses ARC features. If you are using Bugsnag 2.2.0 or newer in your non-arc project, you will need to set a `-fobjc-arc` compiler flag on all of the Bugsnag source files.

To set a compiler flag in Xcode, go to your active target and select the "Build Phases" tab. Now select all Bugsnag source files, press Enter, insert `-fobjc-arc` and then "Done" to enable ARC for Bugsnag.


Reporting Bugs or Feature Requests
----------------------------------

Please report any bugs or feature requests on the github issues page for this project here:

<https://github.com/bugsnag/bugsnag-cocoa/issues>


Contributing
------------

-   [Fork](https://help.github.com/articles/fork-a-repo) the [notifier on github](https://github.com/bugsnag/bugsnag-cocoa)
-   Commit and push until you are happy with your contribution
-   [Make a pull request](https://help.github.com/articles/using-pull-requests)
-   Thanks!


License
-------

The Bugsnag Cocoa notifier is free software released under the MIT License. See [LICENSE.txt](https://github.com/bugsnag/bugsnag-cocoa/blob/master/LICENSE.txt) for details.
