#include <string>
#include <vector>

#include <libjson.h>

namespace toggl {

	typedef std::string error;

	const error noError = "";

	typedef std::string guid;

	class Workspace {
	public:
		long ID;
		std::string Name;

		error Load(JSONNODE *node);
	};

	class Client {
	public:
		long ID;
		guid GUID;
		long WID;
		std::string Name;

		error Load(JSONNODE *node);
	};

	class Project {
	public:
		long ID;
		guid GUID;
		long WID;
		std::string Name;

		error Load(JSONNODE *node);
	};

	class Task {
	public:
		long ID;
		std::string Name;
		long WID;
		long PID;

		error Load(JSONNODE *node);
	};

	class Tag {
	public:
		long ID;
		long WID;
		std::string Name;
		guid GUID;

		error Load(JSONNODE *node);
	};

	class TimeEntry {
	public:
		long ID;
		guid GUID;
		long WID;
		long PID;
		long TID;
		bool Billable;
		std::string Start;
		std::string Stop;
		long DurationInSeconds;
		std::string Description;
		bool DurOnly;
		long UIModifiedAt;

		std::vector<std::string> TagNames;

		error Load(JSONNODE *node);

	private:
		error loadTags(JSONNODE *list);
	};

	class User {
	public:
		long ID;
		std::string APIToken;
		long DefaultWID;

		std::vector<Workspace> Workspaces;
		std::vector<Project> Projects;
		std::vector<Task> Tasks;
		std::vector<TimeEntry> TimeEntries;
		std::vector<Tag> Tags;
		std::vector<Client> Clients;

		// Unix timestamp of the user data; returned from API
		int Since;

		error Fetch();
		error Load(const std::string &json);
		error Load(JSONNODE *node);

	private:
		error loadProjects(JSONNODE *projects);
		error loadTags(JSONNODE *projects);
		error loadClients(JSONNODE *projects);
		error loadTasks(JSONNODE *projects);
		error loadTimeEntries(JSONNODE *projects);
		error loadWorkspaces(JSONNODE *projects);
	};
}

