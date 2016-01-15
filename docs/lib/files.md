# Files in the library

In this section we will give an overview of all the files in the library. We will describe the most essential methods that are described in the files.

## Sitemap

- [User Models](#user-models)
  - [base_model.cc](#base_model.cc)
  - [workspace.cc](#workspace.cc)
  - [user.cc](#user.cc)
  - [time_entry.cc](#time_entry.cc)
  - [project.cc](#project.cc)
  - [client.cc](#client.cc)
  - [tag.cc](#tag.cc)
  - [task.cc](#task.cc)
  - [settings.cc](#settings.cc)
  - [related_data.cc](#related_data.cc)
  - [batch_update_result.cc](#batch_update_result.cc)
  - [model_change.cc](#model_change.cc)
- [Database](#database)
  - [migrations.cc](#migrations.cc)
  - [database.cc](#database.cc)
- [Core API](#core-api)
  - [context.cc](#context.cc)
  - [gui.cc](#gui.cc)
  - [toggl_api.cc](#toggl_api.cc)
  - [toggl_api_private.cc](#toggl_api_private.cc)
- [Connectivity](#connectivity)
  - [https_client.cc](#https_client.cc)
  - [netconf.cc](#netconf.cc)
  - [proxy.cc](#proxy.cc)
  - [websocket_client.cc](#websocket_client.cc)
- [Timeline](#timeline) 
  - [window_change_recorder.cc](#window_change_recorder.cc)
  - [get_focused_window_linux.cc](#get_focused_window_linux.cc)
  - [get_focused_window_mac.cc](#get_focused_window_mac.cc)
  - [get_focused_window_windows.cc](#get_focused_window_windows.cc)
  - [timeline_uploader.cc](#timeline_uploader.cc)
  - [timeline_event.cc](#timeline_event.cc)
- [Utilities](#utilities)
  - [error.cc](#error.cc)
  - [custom_error_handler.cc](#custom_error_handler.cc)
  - [formatter.cc](#formatter.cc)
  - [analytics.cc](#.cc)
  - [urls.cc](#urls.cc)
- [Features](#features)
  - [obm_action.cc](#obm_action.cc)
  - [autotracker.cc](#autotracker.cc)
  - [help_article.cc](#help_article.cc)
  - [feedback.cc](#feedback.cc)
  - [idle.cc](#idle.cc)


## User Models

### base_model.cc

Base model has the base methods needed by all the models used in library. Most of the models inherit from Base model. Most important and non trivial methods will be described in this section.

#### bool BaseModel::NeedsPush() const
    Checks if the model needs a push operation (POST, PUT, DELETE) to be executed

    Note that if a model has a validation error previously received and attached from the backend, the model won't be pushed again unless the error is somehow fixed by user. We will assume that if user modifies the model, the error will go away. But until then, don't push the errored data.

#### bool BaseModel::NeedsPOST() const
    No server side ID yet, meaning it has not been POSTed to the server yet.

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
### user.cc
### time_entry.cc
### project.cc
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
### database.cc

# Core API

### context.cc
### gui.cc
### toggl_api.cc
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
### analytics.cc
### urls.cc

# Features

### obm_action.cc
### autotracker.cc
### help_article.cc
### feedback.cc
### idle.cc
