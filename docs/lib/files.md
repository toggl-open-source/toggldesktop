# Files in the library

In this section we will give an overview of all the files in the library. We will describe the most essential methods that are described in the files.

## Sitemap

- [User Models](#user-models)
  - [base_model.cc](#base_modelcc)
  - [workspace.cc](#workspacecc)
  - [user.cc](#usercc)
  - [time_entry.cc](#time_entrycc)
  - [project.cc](#projectcc)
  - [client.cc](#clientcc)
  - [tag.cc](#tagcc)
  - [task.cc](#taskcc)
  - [settings.cc](#settingscc)
  - [related_data.cc](#related_datacc)
  - [batch_update_result.cc](#batch_update_resultcc)
  - [model_change.cc](#model_changecc)
- [Database](#database)
  - [migrations.cc](#migrationscc)
  - [database.cc](#databasecc)
- [Core API](#core-api)
  - [context.cc](#contextcc)
  - [gui.cc](#guicc)
  - [toggl_api.cc](#toggl_apicc)
  - [toggl_api_private.cc](#toggl_api_privatecc)
- [Connectivity](#connectivity)
  - [https_client.cc](#https_clientcc)
  - [netconf.cc](#netconfcc)
  - [proxy.cc](#proxycc)
  - [websocket_client.cc](#websocket_clientcc)
- [Timeline](#timeline) 
  - [window_change_recorder.cc](#window_change_recordercc)
  - [get_focused_window_linux.cc](#get_focused_window_linuxcc)
  - [get_focused_window_mac.cc](#get_focused_window_maccc)
  - [get_focused_window_windows.cc](#get_focused_window_windowscc)
  - [timeline_uploader.cc](#timeline_uploadercc)
  - [timeline_event.cc](#timeline_eventcc)
- [Utilities](#utilities)
  - [error.cc](#errorcc)
  - [custom_error_handler.cc](#custom_error_handlercc)
  - [formatter.cc](#formattercc)
  - [analytics.cc](#cc)
  - [urls.cc](#urlscc)
- [Features](#features)
  - [obm_action.cc](#obm_actioncc)
  - [autotracker.cc](#autotrackercc)
  - [help_article.cc](#help_articlecc)
  - [feedback.cc](#feedbackcc)
  - [idle.cc](#idlecc)


## User Models

### base_model.cc

Base model has the base methods needed by all the models used in library. Most of the models inherit from Base model. Most important and non trivial methods will be described in this section.

#### bool BaseModel::NeedsPush() const
    Checks if the model needs a push operation (POST, PUT, DELETE) to be executed

    Note that if a model has a validation error previously received and attached from the backend, the model won't be pushed again unless the error is somehow fixed by user. We will assume that if user modifies the model, the error will go away. But until then, don't push the errored data.

#### bool BaseModel::NeedsPOST() const
    No server side ID yet, meaning model has not been POSTed to the server yet.

#### bool BaseModel::NeedsPUT() const
    Model has been updated and is not deleted, needs a PUT

#### bool BaseModel::NeedsDELETE() const
    Model is deleted, needs a DELETE request to the server

#### bool BaseModel::NeedsToBeSaved() const
    Checks if model needs to be saved into local database

#### void BaseModel::EnsureGUID()
    Ensure GUID is set

#### error BaseModel::LoadFromDataString(const std::string data_string)
    Load Data from JSON to model

#### void BaseModel::Delete()
    Delete the model. This triggers DELETE request to the server

#### error BaseModel::ApplyBatchUpdateResult
    Applies update request results to the model

#### std::string BaseModel::batchUpdateRelativeURL() const
    Composes the correct url to be used for model updates

#### error BaseModel::BatchUpdateJSON(Json::Value *result) const
    Convert model JSON into batch update format

#### There are also standard setters as follows:

- SetDeletedAt(const Poco::UInt64 value)
- SetUpdatedAt(const Poco::UInt64 value)
- SetGUID(const std::string value)
- SetUIModifiedAt(const Poco::UInt64 value)
- SetUID(const Poco::UInt64 value)
- SetID(const Poco::UInt64 value)
- SetUpdatedAtString(const std::string value)
- SetDirty()
- SetUnsynced()

### workspace.cc

#### std::string Workspace::String() const
    Return workspace data in a string format
#### void Workspace::SetName(const std::string value)
    Setter for workspace name
#### void Workspace::SetPremium(const bool value)
    Set workspace to be premium (business)
#### void Workspace::SetOnlyAdminsMayCreateProjects(const bool value)
    Setter for workspace setting "Admins may create projects"
#### void Workspace::SetAdmin(const bool value)
    Setter for workspace setting is admin. Mean that user is admin of the workspace or not
#### void Workspace::LoadFromJSON(Json::Value n)
    Load workspace data from JSON to local database
#### std::string Workspace::ModelName() const
    Returns model name in a string
#### std::string Workspace::ModelURL() const
    Returns model update url

### user.cc
### time_entry.cc
### project.cc

#### std::vector<std::string> Project::ColorCodes(known_colors, end(known_colors));
    Color codes for projects
#### std::string Project::String() const
    Return project data in a string format
#### void Project::LoadFromJSON(Json::Value data)
    Load project data from JSON to local database
#### Json::Value Project::SaveToJSON() const
    Convert model data to JSON

#### bool Project::DuplicateResource(const toggl::error err) const
    Returns true if there is a duplicate name error
#### bool Project::ResourceCannotBeCreated(const toggl::error err) const

#### bool Project::clientIsInAnotherWorkspace(const toggl::error err) const

#### bool Project::onlyAdminsCanChangeProjectVisibility(

#### bool Project::ResolveError(const toggl::error err)

#### std::string Project::ModelName() const
    Returns model name in a string
#### std::string Project::ModelURL() const
    Returns model update url

#### Setters as follows:
- void Project::SetClientGUID(const std::string value)
- void Project::SetActive(const #### bool value)
- void Project::SetPrivate(const #### bool value)
- void Project::SetName(const std::string value)
- void Project::SetBillable(const #### bool value)
- void Project::SetColor(const std::string value)
- error Project::SetColorCode(const std::string color_code)
- void Project::SetWID(const Poco::UInt64 value)
- void Project::SetCID(const Poco::UInt64 value)

### client.cc
### tag.cc    
### task.cc       

### settings.cc
### related_data.cc   
### batch_update_result.cc
### model_change.cc

# Database

Toggl Desktop Library uses LiteSQL C++ framework for the local database.

### migrations.cc

Migrations create all the tables and logic of the local database. If anything needs to be changed in the database a new migration should be created to the migrations file.

Example cases:
    - Adding new setting/preference to the app
    - Stroring new user data ( new workspace setting or such )

### database.cc

Database file has all the logic of communicating with the local database. All queries that need to be made in to the local database are described here.

Example cases:
    - Getting user settings
    - Clearing local data (deletes all data connected to the current user ID)

# Core API

### context.cc
### gui.cc

GUI triggers UI actions based on the events triggered either by api itself or the user from the UI. The main aim of the GUI is to keep as much functionality in the library as possible. With GUI we control the UI form the library.

### toggl_api.cc

Toggl Api is the pipe between the UI and the other parts of the library. When UI calls some action `toggl_api` executes the proper method in the `context.cc`.

### toggl_api_private.cc

# Connectivity

### https_client.cc
### netconf.cc
### proxy.cc
### websocket_client.cc

# Timeline 

### window_change_recorder.cc
### get_focused_window_linux.cc
### get_focused_window_mac.cc
### get_focused_window_windows.cc
### timeline_uploader.cc
### timeline_event.cc

# Utilities

### error.cc
### custom_error_handler.cc
### formatter.cc

Formatter object. Used to format time entry texts, dates and more.

### analytics.cc

Analytics object. Formats the analytics data and sends it to google analytics. Currently two different event types are present:
    - GoogleAnalyticsEvent (Simple event)
    - GoogleAnalyticsSettingsEvent (Preferences event, sends preferences values one by once once a day.)

### urls.cc

Url object keeps all the proper urls for requests. Also changes the url between production and staging addresses.

#### void SetUseStagingAsBackend(const bool value)
    Setting a flag so use the stagins urls for requests

#### std::string API()
    Returns API url

#### std::string TimelineUpload()
    Returns timeline url

#### std::string WebSocket()
    Returns websocket url

# Features

### obm_action.cc

Obm actions object. Keeps track of obm experiments. Sets experiment number and keeps track if user has seen an experiment.

### autotracker.cc

Autotracker rule object.
Autotracker has the following parameters:
    - Term (std:string)
    - PID (Poco::UInt64) // project id
    - TID (Poco::UInt64) // task id

#### bool AutotrackerRule::Matches(const TimelineEvent event) const
    Autotracker searches the timeline events for matching the term with event filename or event title.

### help_article.cc

Has collection of all articles in the Toggl Knowlegebase. !This is subject to change in near future.

#### std::vector<HelpArticle> HelpDatabase::GetArticles(const std::string keywords)
    Gets article by seraching the help articles collection by keywords

### feedback.cc

#### const std::string Feedback::filename() const
    Get attached filename

#### toggl::error Feedback::Validate() const
    Validate the that all feedback requirements are met

### idle.cc

#### void Idle::SetIdleSeconds(const Poco::UInt64 idle_seconds, User *current_user)
    Sets idle seconds for user. Is triggered when user changes idle time in preferences.

#### void Idle::computeIdleState(const Poco::UInt64 idle_seconds, User *current_user)
    Compute idle state. If we have been idle for the amount defined in the settings trigger idle notification.
