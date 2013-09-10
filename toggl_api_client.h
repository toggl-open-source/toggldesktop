#include <string>

namespace toggl {

	typedef std::string error;

	const error noError = "";

	typedef std::string guid;

	class Workspace {
	public:
		long ID;
		std::string Name;
	};

	class Project {
	public:
		long ID;
		guid Guid;
		long Wid;
		std::string Name;
	};

	class Task {
	public:
		long ID;
		std::string Name;
		long Wid;
		long Pid;
	};

	class Tag {
	public:
		long ID;
		long Wid;
		std::string Name;
		guid Guid;
	};

	class TimeEntry {
	public:
		long ID;
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

	class User {
	public:
		std::string APIToken;
		long DefaultWid;

		std::string Email;
		std::vector<Workspace*> Workspaces;
		std::vector<Project*> Projects;
		std::vector<Task*> Tasks;
		std::vector<TimeEntry*> TimeEntries;
		std::vector<Tag*> Tags;

		error fetch();
	};
}

