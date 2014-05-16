
XCODE FORMATTER
================

![Why a code formatter](https://raw.github.com/octo-online/Xcode-formatter/master/images/why.png)

A common problem with large iOS projects is the lack of support for a shared set of formatting rules. This leads over the time to unreadable and hard to maintain sources files.

Xcode Formatter uses [uncrustify](http://uncrustify.sourceforge.net/) to easily format your source code as your team exactly wants it to be!

Simply add the __CodeFormatter__ directory in your Xcode project to get : 

* Xcode __shortcut-based code formatting__: a shortcut to format modified sources in the current workspace
* __automatic code formatting__: add a build phase to your project to format current sources when application builds
* __all sources formatting__: format all your code with one command line
* __your formatting rules shared by project__: edit and use a same configuration file with your project dev team

*****

1) How to setup the code formatter for your project
-----------------------------------------------------

### Install uncrustify

The simplest way is to use [brew](http://mxcl.github.com/homebrew/):

	$ brew install uncrustify

To install brew:
 
	$ ruby -e "$(curl -fsSkL raw.github.com/mxcl/homebrew/go)"


### Check that uncrustify is located in /usr/local/bin/ 

	$ which uncrustify

If your uncrustify version is lower than 0.60, you might have to install it manually since modern Objective-C syntax has been added recently, see this post :
http://stackoverflow.com/questions/11862964/uncrustify-doesnt-support-for-the-new-objective-c-syntax-of-llvm-4-0/14202765#14202765

### Add CodeFormatter directory beside your .xcodeproj file

![CodeFormatter directory location](https://raw.github.com/octo-online/Xcode-formatter/master/images/directory_location.png)


### Check that your Xcode application is named "Xcode" (default name)

You can see this name in the Applications/ directory (or your custom Xcode installation directory). Be carefull if you have multiple instances of Xcode on your mac: ensure that project's one is actually named "Xcode"!

(Why this ? This name is used to find currently opened Xcode files. See CodeFormatter/Uncrustify\_opened\_Xcode\_sources.workflow appleScript).


### Install the automator service Uncrustify\_opened\_Xcode\_sources.workflow

Copy this file to your ~/Library/Services/ folder (create this folder if needed).

Be careful : by double-clicking the .workflow file, you will install it but the file will be removed! Be sure to leave a copy of it for other users.




2) How to format opened files when building the project
---------------------------------------------------------

### Add a build phase "run script" containing the following line:

	sh CodeFormatter/scripts/formatOpenedSources.sh

![Add build phase](https://raw.github.com/octo-online/Xcode-formatter/master/images/add_build_phase.png)




3) How to format opened files with a shortcut
-----------------------------------------------

### Add a shortcut to the Uncrustify_opened_Xcode_sources service

Go to Xcode > Services > Services preferences :

![Create service shortcut](https://raw.github.com/octo-online/Xcode-formatter/master/images/add_service_shortcut.png)

And set your shortcut :

![Choose service shortcut](https://raw.github.com/octo-online/Xcode-formatter/master/images/choose_service_shortcut.png)




4) How to format files in command line
----------------------------------------

### To format currently opened files, use formatOpenedSources.sh: 

	$ sh CodeFormatter/scripts/formatOpenedSources.sh


### To format all files, use formatAllSources.sh:

	$ sh CodeFormatter/scripts/formatAllSources.sh PATH

PATH must be replaced by your sources path.



5) How to change formatter’s rules
------------------------------------

### Edit CodeFormatter/uncrustify\_objective\_c.cfg

You can use UniversalIndentGUI (http://universalindent.sourceforge.net/) to simplify edition.



6) FAQ
--------

### When formatting the current sources, an unknown error occurs and shows my code. What can I do? 

Uncrustify (version 0.59) does not handle apple new literals for creating NSDictionary and NSArray like: 

	NSDictionary *dictionary = @{@0 : @"red",
	 							 @1 : @"green",
	 							 @2 : @"blue"};

Until a new version of uncrustify fixes this issue, you can use the old way with those literals:

	NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:@"red", @1, 
	                                                                       @"green", @2, 
	                                                                       @"blue", @3, 
	                                                                       nil];

### I want to have a single configuration file for all my Xcode projects, is it possible?

Yes, everything is possible. Just move your configuration file to your home directory: '~/.uncrustify//uncrustify\_objective\_c.cfg' and remove 'CodeFormatter' directory from your project. If the configuration file is not found in project directory, this global file is used.
