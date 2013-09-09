
#include <string>
#include <vector>
#include <sstream>

#include "Poco/Logger.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"

namespace toggl {

	typedef std::string error;

	const int nil = 0;
	const error noError = "";

	typedef std::string guid;

	class BaseModel {
	public:

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

		error fetch() {
			poco_assert(!APIToken.empty());
			Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
			try {
				const Poco::URI uri("https://www.toggl.com");
				const Poco::Net::Context::Ptr context(new Poco::Net::Context(
					Poco::Net::Context::CLIENT_USE, "", "", "",
					Poco::Net::Context::VERIFY_NONE, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
				Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
				session.setKeepAlive(false);

				Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
					"/api/v8/me?with_related_data=true", Poco::Net::HTTPMessage::HTTP_1_1);
				req.setKeepAlive(false);

				logger.debug("Sending request..");

				Poco::Net::HTTPBasicCredentials cred(APIToken, "api_token");
				cred.authenticate(req);
				session.sendRequest(req) << std::flush;

				std::stringstream request_string;
				req.write(request_string);
				logger.debug(request_string.str());

				logger.debug("Request sent. Receiving response..");

				Poco::Net::HTTPResponse response;
				std::istream& is = session.receiveResponse(response);

				std::stringstream response_string;
				response_string << "Response received: " << response.getStatus() << " " << response.getReason();
				logger.debug(response_string.str());

				std::ostringstream body;
				Poco::StreamCopier::copyStream(is, body);

				if ((response.getStatus() != 202) && (response.getStatus() != 200)) {
					return body.str();
				}

				std::string json = body.str();
				logger.debug(json);

			} catch (const Poco::Exception& exc) {
				logger.error(exc.displayText());
				return exc.displayText();
			} catch (const std::exception& ex) {
				logger.error(ex.what());
				return ex.what();
			} catch (const std::string& ex) {
				logger.error(ex);
				return ex;
			}

			return noError;
		}
	};
}