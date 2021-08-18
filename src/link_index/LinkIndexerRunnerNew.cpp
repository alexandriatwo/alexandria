
#include "LinkIndexerRunnerNew.h"
#include "LinkIndexer.h"
#include <math.h>
#include "system/Logger.h"
#include "full_text/FullText.h"

LinkIndexerRunnerNew::LinkIndexerRunnerNew(const string &db_name, const string &domain_db_name, const string &hash_table_name,
	const string &domain_hash_table_name, const string &cc_batch, const SubSystem *sub_system, UrlToDomain *url_to_domain)
: m_cc_batch(cc_batch), m_db_name(db_name), m_domain_db_name(domain_db_name), m_hash_table_name(hash_table_name),
	m_domain_hash_table_name(domain_hash_table_name)
{
	m_sub_system = sub_system;
	m_url_to_domain = url_to_domain;
}

LinkIndexerRunnerNew::~LinkIndexerRunnerNew() {
}

void LinkIndexerRunnerNew::run(size_t partition, size_t max_partitions) {

	//truncate();

	TsvFileRemote warc_paths_file(string("crawl-data/") + m_cc_batch + "/warc.paths.gz");

	vector<string> warc_paths_raw;
	warc_paths_file.read_column_into(0, warc_paths_raw);

	vector<string> warc_paths = FullText::make_partition_from_files(warc_paths_raw, partition, max_partitions);

	vector<vector<string>> warc_path_chunks;
	vector_chunk(warc_paths, ceil((float)warc_paths.size() / LI_NUM_THREADS_INDEXING), warc_path_chunks);

	ThreadPool pool(LI_NUM_THREADS_INDEXING);
	std::vector<std::future<string>> results;

	int id = 1;
	for (const vector<string> &warc_paths_chunk : warc_path_chunks) {

		results.emplace_back(
			pool.enqueue([this, warc_paths_chunk, id] {
				return run_index_thread(warc_paths_chunk, id);
			})
		);

		id++;

	}

	for(auto && result: results) {
		result.get();
	}

	merge();
	sort();

}

void LinkIndexerRunnerNew::merge() {
	LogInfo("Merging...");
	Profiler profiler("Merging...");

	const size_t merge_batch_size = 250;

	ThreadPool merge_pool(LI_NUM_THREADS_MERGING);
	std::vector<std::future<string>> merge_results;

	// Loop over shards and merge them.
	for (size_t shard_id = 0; shard_id < FT_NUM_SHARDS; ) {

		while (shard_id < FT_NUM_SHARDS && merge_results.size() < merge_batch_size) {

			merge_results.emplace_back(
				merge_pool.enqueue([this, shard_id] {
					return run_merge_thread(shard_id);
				})
			);

			shard_id++;

		}

		for (auto && result: merge_results) {
			result.get();
		}
		merge_results.clear();
	}

}

void LinkIndexerRunnerNew::merge_adjustments() {
	LogInfo("Merging adjustments...");
	Profiler profiler("Merging adjustments...");

	/*
	const size_t merge_batch_size = 250;

	ThreadPool merge_pool(LI_NUM_THREADS_MERGING);
	std::vector<std::future<string>> merge_results;

	FullTextIndexer *indexer = new FullTextIndexer(1, "main_index", m_sub_system, m_url_to_domain);

	// Loop over shards and merge them.
	for (size_t shard_id = 0; shard_id < FT_NUM_SHARDS; ) {

		while (shard_id < FT_NUM_SHARDS && merge_results.size() < merge_batch_size) {
			merge_results.emplace_back(
				merge_pool.enqueue([this, indexer, shard_id] {
					return run_merge_adjustments_thread(indexer, shard_id);
				})
			);

			shard_id++;
		}

		for (auto && result: merge_results) {
			result.get();
		}

		merge_results.clear();

	}

	delete indexer;
	*/
}

void LinkIndexerRunnerNew::sort() {
	LogInfo("Sorting...");
	Profiler profiler("Sorting...");

	// Loop over hash table shards and merge them.
	for (size_t shard_id = 0; shard_id < HT_NUM_SHARDS; shard_id++) {
		HashTableShardBuilder *shard = new HashTableShardBuilder(m_hash_table_name, shard_id);
		shard->sort();
		delete shard;
	}
	for (size_t shard_id = 0; shard_id < HT_NUM_SHARDS; shard_id++) {
		HashTableShardBuilder *shard = new HashTableShardBuilder(m_domain_hash_table_name, shard_id);
		shard->sort();
		delete shard;
	}
}

void LinkIndexerRunnerNew::truncate() {
	for (size_t shard_id = 0; shard_id < FT_NUM_SHARDS; shard_id++) {
		FullTextShardBuilder<LinkFullTextRecord> *shard_builder =
			new FullTextShardBuilder<LinkFullTextRecord>(m_db_name, shard_id);
		shard_builder->truncate();
		delete shard_builder;
	}
}

void LinkIndexerRunnerNew::index_stream(ifstream &infile) {

	vector<HashTableShardBuilder *> shard_builders;
	for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
		shard_builders.push_back(new HashTableShardBuilder(m_hash_table_name, i));
	}

	LinkIndexer indexer(1, m_db_name, m_sub_system, m_url_to_domain);
	indexer.add_stream(shard_builders, infile);

	indexer.flush_cache(m_link_mutexes);

	for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
		shard_builders[i]->write();
	}

	for (HashTableShardBuilder *shard_builder : shard_builders) {
		delete shard_builder;
	}
}

string LinkIndexerRunnerNew::run_index_thread(const vector<string> &warc_paths, int id) {

	vector<HashTableShardBuilder *> shard_builders;
	for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
		shard_builders.push_back(new HashTableShardBuilder(m_hash_table_name, i));
	}

	vector<HashTableShardBuilder *> domain_shard_builders;
	for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
		domain_shard_builders.push_back(new HashTableShardBuilder(m_domain_hash_table_name, i));
	}

	LinkIndexer indexer(id, m_db_name, m_sub_system, m_url_to_domain);
	LinkIndexer domain_link_indexer(id, m_domain_db_name, m_sub_system, m_url_to_domain);
	size_t idx = 1;
	for (const string &raw_warc_path : warc_paths) {

		string warc_path = raw_warc_path;
		warc_path.replace(warc_path.find(".warc.gz"), 8, ".links.gz");

		int error;
		const string data = Transfer::gz_file_to_string(warc_path, error);
		if (error == Transfer::OK) {
			{
				stringstream stream(data);
				indexer.add_stream(shard_builders, stream);
				indexer.write_cache(m_link_mutexes);
			}
			{
				stringstream stream(data);
				domain_link_indexer.add_domain_link_stream(domain_shard_builders, stream);
				domain_link_indexer.write_cache(m_domain_link_mutexes);
			}
		}

		for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
			if (shard_builders[i]->full()) {
				m_hash_table_mutexes[i].lock();
				shard_builders[i]->write();
				m_hash_table_mutexes[i].unlock();
			}
		}

		for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
			if (domain_shard_builders[i]->full()) {
				m_domain_hash_table_mutexes[i].lock();
				domain_shard_builders[i]->write();
				m_domain_hash_table_mutexes[i].unlock();
			}
		}

		LogInfo("Done " + to_string(idx) + " out of " + to_string(warc_paths.size()));

		idx++;
	}
	indexer.flush_cache(m_link_mutexes);
	domain_link_indexer.flush_cache(m_domain_link_mutexes);

	for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
		m_hash_table_mutexes[i].lock();
		shard_builders[i]->write();
		m_hash_table_mutexes[i].unlock();
	}

	for (size_t i = 0; i < HT_NUM_SHARDS; i++) {
		m_domain_hash_table_mutexes[i].lock();
		domain_shard_builders[i]->write();
		m_domain_hash_table_mutexes[i].unlock();
	}

	for (HashTableShardBuilder *shard_builder : shard_builders) {
		delete shard_builder;
	}

	for (HashTableShardBuilder *domain_shard_builder : domain_shard_builders) {
		delete domain_shard_builder;
	}

	return "";
}

string LinkIndexerRunnerNew::run_merge_thread(size_t shard_id) {

	/*
	FullTextShardBuilder<FullTextRecord> adjustment_shard("adjustments", shard_id);
	adjustment_shard.merge();

	FullTextShardBuilder<FullTextRecord> domain_adjustment_shard("domain_adjustments", shard_id);
	domain_adjustment_shard.merge();*/

	{
		FullTextShardBuilder<LinkFullTextRecord> shard(m_db_name, shard_id);
		shard.merge();
	}

	{
		FullTextShardBuilder<LinkFullTextRecord> shard(m_domain_db_name, shard_id);
		shard.merge();
	}

	return "";
}

string LinkIndexerRunnerNew::run_merge_adjustments_thread(const FullTextIndexer *indexer, size_t shard_id) {

	/*
	FullTextShardBuilder<FullTextRecord> shard1("main_index", shard_id);
	FullTextShardBuilder<FullTextRecord> shard2("adjustments", shard_id);
	FullTextShardBuilder<FullTextRecord> shard3("domain_adjustments", shard_id);

	shard1.merge_domain(shard2, shard3, indexer->url_to_domain());

	LogInfo("Merged " + to_string(shard_id));

	return shard1.filename();*/
	return "";
}

