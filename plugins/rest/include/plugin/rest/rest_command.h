/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include <string>

namespace Link {

class RestCommand {
public:
	RestCommand() : m_prev(0), m_next(0), m_id(0) {}
	virtual ~RestCommand(){}

	/// Command name getter.
	/// @return name of the command.
	virtual const char* Name() const = 0;

	/// Command processor method to be implemented by derived classes.
	/// @param query_string A raw unescaped copy of the query string from the
	/// requester. A zero length query string implies that there was no query
	/// string in the original request.
	/// @param post_data A complete copy of the POST request body. If the
	/// request was a GET then the post_data will be an empty string.
	/// @param response_data A string that should be populated with results
	/// that should be sent back to the requester.
	/// @return The command processor should return true if the command
	/// processing succeeded and false if a failure occurred. Regardless of the
	/// return value, any data written to response_data will be sent to
	/// the requester.
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data) = 0;
 private:
	RestCommand* m_prev;
	RestCommand* m_next;
	int m_id;
	friend class RestClient;
};

} // namespace Link
