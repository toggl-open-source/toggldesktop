#include <string>
#include <vector>

namespace toggl {

	typedef std::string* error;

	const int nil = 0;

	typedef std::string guid;

	class BaseModel {
	public:
		long ID;

		error Save() {
			return nil;
		};

		error Load() {
			return nil;
		};
	};

	class Workspace : public BaseModel {
	public:
		std::string Name;
	};

	class Project : public BaseModel {
	public:
		guid Guid;
		long Wid;
		std::string Name;
	};

	class Task : public BaseModel {
	public:
		std::string Name;
		long Wid;
		long Pid;
	};

	class Tag : public BaseModel {
	public:
		long Wid;
		std::string Name;
		guid Guid;
	};

	class TimeEntry : public BaseModel {
	public:
		guid Guid;
		long Wid;
		long Pid;
		long Tid;
		bool Billable;
		long Start;
		long Stop;
		long DurationInSeconds;
		std::string Description;
		std::vector<std::string> TagNames;
		bool DurOnly;
		long UiModifiedAt;
	};

	class User : public BaseModel {
	public:
		std::string APIToken;
		long DefaultWid;

		std::string Email;
		std::vector<Workspace*> Workspaces;
		std::vector<Project*> Projects;
		std::vector<Task*> Tasks;
		std::vector<TimeEntry*> TimeEntries;
		std::vector<Tag*> Tags;
	};
}