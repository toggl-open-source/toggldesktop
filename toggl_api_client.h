#include <string>
#include <vector>

#include <libjson.h>

#include "types.h"
#include "database.h"

namespace kopsik {

	class Workspace {
	public:
		Workspace() : ID(0), Name("") {}

		long ID;
		std::string Name;

		error Load(JSONNODE *node);
	};

	class Client {
	public:
		Client() : ID(0), GUID(""), WID(0), Name("") {}

		long ID;
		guid GUID;
		long WID;
		std::string Name;

		error Load(JSONNODE *node);
	};

	class Project {
	public:
		Project() : ID(0), GUID(""), WID(0), CID(0), Name("") {}

		long ID;
		guid GUID;
		long WID;
		long CID;
		std::string Name;

		error Load(JSONNODE *node);
	};

	class Task {
	public:
		Task() : ID(0), Name(""), WID(0), PID(0) {}

		long ID;
		std::string Name;
		long WID;
		long PID;

		error Load(JSONNODE *node);
	};

	class Tag {
	public:
		Tag() : ID(0), WID(0), Name(""), GUID("") {}

		long ID;
		long WID;
		std::string Name;
		guid GUID;

		error Load(JSONNODE *node);
	};

	class TimeEntry {
	public:
		TimeEntry() : ID(0), GUID(""), WID(0), PID(0), TID(0), Billable(false), 
			Start(""), Stop(""), DurationInSeconds(0), Description(""),
			DurOnly(false), UIModifiedAt(0) {}

		long ID;
		guid GUID;
		long WID;
		long PID;
		long TID;
		bool Billable;
		// FIXME: should be long
		std::string Start;
		std::string Stop;
		long DurationInSeconds;
		std::string Description;
		bool DurOnly;
		long UIModifiedAt; // TE is the only model that can actually be updated by user.

		std::vector<std::string> TagNames;

		error Load(JSONNODE *node);

	private:
		error loadTags(JSONNODE *list);
	};

	class User {
	public:
		User() : ID(0), APIToken(""), DefaultWID(0), Since(0) {}

		long ID;
		std::string APIToken;
		long DefaultWID;

		// Unix timestamp of the user data; returned from API
		long Since;

		std::vector<Workspace> Workspaces;
		std::vector<Project> Projects;
		std::vector<Task> Tasks;
		std::vector<TimeEntry> TimeEntries;
		std::vector<Tag> Tags;
		std::vector<Client> Clients;

		error Fetch();
		error Load(const std::string &json);
		error Load(JSONNODE *node);
		error Save(Database &db);

	private:
		error loadProjects(JSONNODE *projects);
		error loadTags(JSONNODE *projects);
		error loadClients(JSONNODE *projects);
		error loadTasks(JSONNODE *projects);
		error loadTimeEntries(JSONNODE *projects);
		error loadWorkspaces(JSONNODE *projects);
	};
}

