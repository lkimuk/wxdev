#ifndef WXDEV_UTILS_HPP
#define WXDEV_UTILS_HPP

#include "sha1.hpp"
#include <curl/curl.h>
#include <string_view>
#include <vector>
#include <algorithm>


namespace wxdev {


	// check if token is valid
	bool check(std::string_view signature, std::string_view token,
		std::string_view timestamp, std::string_view nonce) {
		if (signature.empty()) return false;

		std::vector<std::string_view> params{ token, timestamp, nonce };

		std::sort(params.begin(), params.end());
		std::string checktext;
		for (const auto& elem : params)
			checktext.append(elem);
		//std::cout << checktext << std::endl;
		SHA1 hash;
		hash.update(checktext);
		const std::string hashtext = hash.final();
		std::cout << hashtext << std::endl;

		return signature == hashtext ? true : false;
	}


	std::string get_redirect_url(const std::string& url) {
		CURL* curl = nullptr;
		CURLcode res = CURLE_OK;
		curl = curl_easy_init();
		std::string response;
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			res = curl_easy_perform(curl);
			CURLcode code;
			char* redirect_url = nullptr;
			long res_code;
			code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
			if(res_code == 404) return "";
			
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			code = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirect_url);
			if (redirect_url) {
				response = redirect_url;
			}
		}

		curl_easy_cleanup(curl);
		return response;
	}


} // namespace okwedev

#endif // WEDEV_UTILS_HPP