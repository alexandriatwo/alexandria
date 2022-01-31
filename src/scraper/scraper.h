/*
 * MIT License
 *
 * Alexandria.org
 *
 * Copyright (c) 2021 Josef Cullhed, <info@alexandria.org>, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <queue>
#include <curl/curl.h>
#include "robots.h"
#include "store.h"
#include "parser/URL.h"
#include "urlstore/DomainData.h"

namespace Scraper {

	std::string user_agent_token();
	std::string user_agent();

	/*
	 * The scraper!
	 * */
	class scraper {
		public:

			scraper(const std::string &domain, store *store);
			~scraper();

			void push_url(const URL &url);
			void run();
			void start_thread();
			bool finished();

		private:
			std::thread m_thread;
			bool m_finished = false;
			std::string m_domain;
			std::string m_buffer;
			size_t m_buffer_len = 1024*1024*10;
			CURL *m_curl;
			store *m_store;
			std::queue<URL> m_queue;
			googlebot::RobotsMatcher m_robots;
			UrlStore::DomainData m_domain_data;
			std::string m_robots_content;
			size_t m_num_total = 0;
			size_t m_num_www = 0;
			size_t m_num_https = 0;

			void handle_error(const std::string &error);
			void handle_url(const URL &url);
			void mark_all_urls_with_error(size_t error_code);
			void update_url(const URL &url, size_t http_code, size_t last_visited, const URL &redirect);
			void handle_response(const std::string &data, size_t response_code, const std::string &ip, const URL &url);
			void download_domain_data();
			void download_robots();
			bool robots_allow_url(const URL &url) const;
			std::string simple_get(const URL &url);
			void upload_domain_info();
			URL filter_url(const URL &url);

		public:

			friend size_t curl_string_reader(char *ptr, size_t size, size_t nmemb, void *userdata);
	};

	size_t curl_string_reader(char *ptr, size_t size, size_t nmemb, void *userdata);

	void url_downloader();
	void run_scraper_on_urls(const std::vector<std::string> &input_urls);

}
