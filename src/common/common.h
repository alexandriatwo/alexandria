
#pragma once

#include <aws/core/Aws.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <vector>

#include "system/Profiler.h"

#define CC_ERROR 1
#define CC_OK 0

template<class T> void vector_chunk(const std::vector<T> &vec, int chunk_size, std::vector<std::vector<T>> &dest);
template<class T> void vector_chunk(const std::vector<T> &vec, int chunk_size, std::vector<std::vector<T>> &dest) {
	std::vector<T> chunk;
	for (T item : vec) {
		chunk.push_back(item);
		if (chunk.size() == chunk_size) {
			dest.push_back(chunk);
			chunk.clear();
		}
	}
	if (chunk.size()) {
		dest.push_back(chunk);
	}
}

std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> get_logger_factory();