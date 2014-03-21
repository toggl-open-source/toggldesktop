// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_COMMON_NOTIFICATIONS_H_
#define SRC_UI_COMMON_NOTIFICATIONS_H_

#include <string>

#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"

namespace kopsik {

class UserLoggedInNotification : public Poco::Notification {};

class UserLoggedOutNotification : public Poco::Notification {};

class TimerRunningNotification : public Poco::Notification {};

class TimerStoppedNotification : public Poco::Notification {};

class TimeEntrySelectedNotification : public Poco::Notification {};

class TimeEntryDeselectedNotification : public Poco::Notification {};

class ErrorNotification : public Poco::Notification {};

class UpdateAvailableNotification : public Poco::Notification {};

class UpToDateNotification : public Poco::Notification {};

class ModelChangeNotification : public Poco::Notification {};

class IdleFinishedNotification : public Poco::Notification {};

class SettingsChangedNotification : public Poco::Notification {};

class ShowListViewNotification : public Poco::Notification {};

class NewNotification : public Poco::Notification {};

class StopNotification : public Poco::Notification {};

class ContineNotification : public Poco::Notification {};

class ShowPreferencesNotification : public Poco::Notification {};

class StopAtNotification : public Poco::Notification {};

class SplitAtNotification : public Poco::Notification {};

class EditRunningTimeEntryNotification : public Poco::Notification {};

class OfflineNotification : public Poco::Notification {};

class OnlineNotification : public Poco::Notification {};

}  // namespace kopsik

#endif  // SRC_UI_COMMON_NOTIFICATIONS_H_
