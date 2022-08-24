/**
 * @fileOverview SearchResults class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.SearchResults = (function() {
   const prototype = {};
   const privates = new WeakMap();

   hbbtv.utils.defineGetterProperties(prototype, {
      length() {
         return privates.get(this).searchResultsData.results.length;
      },
      offset() {
         return privates.get(this).searchResultsData.offset;
      },
      totalSize() {
         return privates.get(this).searchResultsData.totalSize;
      }
   });

   prototype.item = function(index) {
      const p = privates.get(this);
      if (index < 0 || p.searchResultsData.results.length <= index) {
         return undefined;
      }
      return hbbtv.objects.createProgramme(p.searchResultsData.results[index]);
   }

   prototype.getResults = function(offset, count) {
      const p = privates.get(this);
      p.searchResultsData.results = [];
      const metadataSearch = (p.metadataSearch && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? p.metadataSearch.deref() : p.metadataSearch;
      /* Note: state -1 means the search has been started */
      hbbtv.objects.MetadataSearch.dispatchMetadataSearch.call(metadataSearch, p.searchResultsData.query.queryId(), -1, null, offset, count);
      return false;
   }

   prototype.abort = function() {
      const p = privates.get(this);
      hbbtv.bridge.broadcast.abortSearch(p.searchResultsData.query.queryId());
      p.searchResultsData.results = [];
      const metadataSearch = (p.metadataSearch && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? p.metadataSearch.deref() : p.metadataSearch;
      /* Note: state 3 means the search has been aborted */
      hbbtv.objects.MetadataSearch.dispatchMetadataSearch.call(metadataSearch, p.searchResultsData.query.queryId(), 3, null);
   }

   function initialise(searchResultsData, metadataSearch) {
      privates.set(this, {});
      const p = privates.get(this);
      p.searchResultsData = searchResultsData;
      p.searchResultsData.results = [];
      p.searchResultsData.offset = 0;
      p.searchResultsData.totalSize = 0;
      p.metadataSearch = (metadataSearch && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? new WeakRef(metadataSearch) : metadataSearch;
   }

   function setResults(results, offset, totalSize) {
      const p = privates.get(this);
      p.searchResultsData.results = results;
      p.searchResultsData.offset = offset;
      p.searchResultsData.totalSize = totalSize;
   }

   return {
      prototype: prototype,
      initialise: initialise,
      setResults: setResults
   };
})();

hbbtv.objects.SearchResultsProxy = {
   get: function(target, name, receiver) {
      if (name === "setResults") {
         const origMethod = hbbtv.objects.SearchResults.setResults;
         return function(...args) {
            return origMethod.apply(target, args);
         };
      }
      if (name in target) {
         const origMethod = target[name];
         if (origMethod instanceof Function) {
            return function(...args) {
               return origMethod.apply(target, args);
            };
         }
         return target[name];
      }
      if (typeof name === 'string' || name instanceof String) {
         const index = parseInt(name);
         if (!isNaN(index)) {
            return target.item(index);
         }
      }
   }
};

hbbtv.objects.createSearchResults = function(searchResultsData, metadataSearch) {
   const searchResults = Object.create(hbbtv.objects.SearchResults.prototype);
   hbbtv.objects.SearchResults.initialise.call(searchResults, searchResultsData, metadataSearch);
   return new Proxy(searchResults, hbbtv.objects.SearchResultsProxy);
}