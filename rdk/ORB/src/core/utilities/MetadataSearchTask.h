/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <core/core.h>
#include <memory>
#include <string>
#include <vector>
#include "Programme.h"
#include "Query.h"

// Supported search status values
#define SEARCH_STATUS_COMPLETED   0
#define SEARCH_STATUS_ABORTED     3
#define SEARCH_STATUS_NO_RESOURCE 4

namespace orb {
using namespace WPEFramework::Core;

/**
 * Implements the metadata search task that is used to filter out programmes
 * based on criteria set in a Query. The metadata search task shall run asynchronously
 * in its own dedicated thread. The search results shall be sent to the JavaScript
 * context asynchronously by means of the 'MetadataSearch' bridge event.
 */
class MetadataSearchTask : public Thread {
public:

   /**
    * Constructor.
    *
    * @param query              Shared pointer to the query
    * @param offset             The specified offset for the search results
    * @param count              The specified count for the search results
    * @param channelConstraints The additional channel constraints
    */
   MetadataSearchTask(std::shared_ptr<Query> query, int offset, int count, std::vector<std::string> channelConstraints);

   /**
    * Destructor.
    */
   virtual ~MetadataSearchTask();

   /**
    * Dispatch the MetadataSearch bridge event to the current page's JavaScript context.
    *
    * @param search        The search id
    * @param status        0 (Completed) or 3 (Aborted) or 4 (No resource found)
    * @param searchResults The list of JSON programme objects that match the search criteria
    */
   static void OnMetadataSearchCompleted(int search, int status, std::vector<std::string> searchResults);

   /**
    * @brief MetadataSearch::Worker
    *
    * Worker method that runs in a dedicated thread.
    *
    * @return infinite
    */
   virtual uint32_t Worker() override;

   /**
    * Stop the search task thread.
    */
   void Stop();

private:

   /**
    * @brief MetadataSearchTask::Match
    *
    * Matches the specified programme against the specified query.
    *
    * @param query     The specified query
    * @param programme The specified programme
    * @param ccid      The ID of the channel that the specified programme belongs to
    *
    * @return true if the programme matches the query, or else false
    */
   bool Match(std::shared_ptr<Query> query, Programme programme, std::string ccid);

   /**
    * @brief MetadataSearchTask::CompareStringValues
    *
    * Compare the given programme and query string values.
    *
    * @param comparison     The comparison type
    * @param programmeValue The programme value
    * @param queryValue     The query value
    *
    * @return true if the programme and query values match, or else false
    */
   bool CompareStringValues(Query::Comparison comparison, std::string programmeValue, std::string queryValue);

   /**
    * @brief MetadataSearchTask::CompareLongValues
    *
    * Compare the given programme and query long values.
    *
    * @param comparison     The comparison type
    * @param programmeValue The programme value
    * @param queryValue     The query value
    *
    * @return true if the programme and query values match, or else false
    */
   bool CompareLongValues(Query::Comparison comparison, long programmeValue, long queryValue);

   // member variables
   std::shared_ptr<Query> m_query;
   int m_offset;
   int m_count;
   std::vector<std::string> m_channelConstraints;
   std::vector<std::string> m_searchResults;
}; // class MetadataSearchTask
} // namespace orb
