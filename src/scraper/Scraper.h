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

#include <queue>
#include <curl/curl.h>
#include "parser/URL.h"

namespace Scraper {

	std::string user_agent_token();
	std::string user_agent();

	/*
	 * Responsible for storing scraper data on a file and upload it to our fileserver when the file reaches a number of urls.
	 * */
	class store {
		public:

	};

	/*
	 * The scraper!
	 * */
	class scraper {
		public:

			scraper(const std::string &domain, store *store);
			~scraper();

			void push_url(const URL &url);
			void run();

		private:
			std::string m_domain;
			std::string m_buffer;
			size_t m_buffer_len = 1024*1024*10;
			CURL *m_curl;
			store *m_store;
			std::queue<URL> m_queue;

			void handle_url(const URL &url);
			void handle_response(const std::string &data, size_t response_code, const URL &url);

		public:

			friend size_t curl_string_reader(char *ptr, size_t size, size_t nmemb, void *userdata);
	};

	size_t curl_string_reader(char *ptr, size_t size, size_t nmemb, void *userdata);

}
