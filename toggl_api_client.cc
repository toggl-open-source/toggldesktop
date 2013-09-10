#include <string>
#include <vector>
#include <sstream>

#include "toggl_api_client.h"

#include "Poco/Bugcheck.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"

namespace toggl {

error User::fetch() {
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
	};
}
