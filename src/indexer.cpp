
#include <iterator>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>

#include <iostream>
#include <memory>
#include <unistd.h>

#include "CCFullTextIndexer.h"
#include "CCUrlIndex.h"
#include "CCIndexRunner.h"
#include "CCLinkIndex.h"

using namespace std;
using namespace Aws::Utils::Json;

namespace io = boost::iostreams;

int main(int argc, const char **argv) {

	//CCFullTextIndexer::run_all();
	//CCUrlIndexer::run_all();
	//CCIndexRunner<CCUrlIndex> indexer;
	CCIndexRunner<CCLinkIndex> indexer;
	indexer.run_all();

	return 0;
}

