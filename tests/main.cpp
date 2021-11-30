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

#define BOOST_TEST_MODULE "Unit tests for alexandria.org"

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include "config.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <streambuf>
#include <math.h>
#include "search_engine/SearchAllocation.h"

using namespace std;

#include "search_allocation.h"
#include "file.h"
#include "url.h"
#include "html_parser.h"
#include "unicode.h"
#include "text.h"
#include "sub_system.h"
#include "hash_table.h"
#include "full_text.h"
#include "invoke.h"
#include "api.h"
#include "search_engine.h"
#include "configuration.h"
#include "performance.h"
#include "sort.h"
#include "algorithm.h"
#include "deduplication.h"
#include "sections.h"
#include "logger.h"
#include "hyper_log_log.h"
#include "hyper_ball.h"
#include "cluster.h"

