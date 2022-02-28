#ifndef WXDEV_MESSAGE_HANDLER_HPP
#define WXDEV_MESSAGE_HANDLER_HPP

#include "pugixml.hpp"
#include <unordered_map>
#include <functional>
#include <chrono>
#include <sstream>



namespace wxdev {



struct response {

	enum response_body {
		content,
		type
	};
	
	// writes response value
	void set(response_body type, const char* value) {
		auto key = get_response_body(type);
		pugi::xml_node root = doc_.child("xml");
		root.append_child(key).text().set(value);
	}

private:
	response() = default;

	// initializes response message
	void initialize(const pugi::xml_node& request) {
		pugi::xml_node root = doc_.append_child("xml");
		auto to_user_name = request.child("FromUserName").text();
		auto from_user_name = request.child("ToUserName").text();
		using clock_type = std::chrono::system_clock;
		clock_type::time_point pt = clock_type::now();
		std::chrono::nanoseconds d = pt.time_since_epoch();
		std::chrono::milliseconds millsec = std::chrono::duration_cast<std::chrono::milliseconds>(d);

		root.append_child("ToUserName").text().set(to_user_name);
		root.append_child("FromUserName").text().set(from_user_name);
		root.append_child("CreateTime").text().set(millsec.count());
		root.append_child("MsgType").text().set("text"); // Ĭ�ϻظ�����Ϊtext
	}


	// get string of response_type
	const char* get_response_body(response_body type) {
		static std::unordered_map<response_body, const char*> types{
			{response_body::content, "Content"},
			{response_body::type, "MsgType"}
		};

		auto p = types.find(type);
		if (p == types.end())
			throw std::runtime_error("invalid content type!");

		return p->second;
	}

	std::string data() {
		std::stringstream ss;
		doc_.save(ss, " ");
		return ss.str();
	}

	friend class message_handler;

private:
	pugi::xml_document doc_;
};


class message_handler {

	using callback_type = std::function<void(response&)>;

public:
	message_handler(const char* message) {
		doc_.load_string(message);
		res_.initialize(doc_.child("xml"));
	}

	// 
	// size_t size() {
	// 	return 1;
	// }

	// register message handler
	void set_handler(const std::string& type, callback_type callback) {
		handler_[type] = callback;
	}

	// get value by key
	std::string get_value(const char* key) {
		pugi::xml_node node = doc_.child("xml");
		return node.child(key).text().as_string();
	}

	// return message type
	std::string get_type() {
		pugi::xml_node node = doc_.child("xml");
		return node.child("MsgType").text().as_string();
	}

	std::string data() {
		auto p = handler_.find(get_type());
		if (p == handler_.end()) {
			// throw std::runtime_error("error! failed to handle [" + get_type() + "] message.");
			res_.set(response::content, "failed to handle this message");
			return res_.data();
		}
		(p->second)(res_);

		return res_.data();
	}


private:
	pugi::xml_document doc_;
	response res_;
	std::unordered_map<std::string, callback_type> handler_;
};

} // namespace wxdev

#endif // WXDEV_MESSAGE_HANDLER_HPP