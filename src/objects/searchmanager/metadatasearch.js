/**
 * @fileOverview MetadataSearch class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

/*jshint esversion: 6 */

hbbtv.objects.MetadataSearch = (function() {
   const prototype = {};
   const privates = new WeakMap();

   const State = Object.freeze({
      IDLE: 0,
      SEARCHING: 1,
      FOUND: 2
   });

   hbbtv.utils.defineGetterProperties(prototype, {
      searchTarget() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return p.searchTarget;
      },
      result() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return p.result;
      }
   });

   prototype.setQuery = function(query) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      internalSetQuery.call(this, query, p);
   };

   prototype.addChannelConstraint = function(channel) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      if (channel === null) {
         p.channelConstraints = [];
      } else {
         if (p.channelConstraints.indexOf(channel.ccid) === -1) {
            p.channelConstraints.push(channel.ccid);
         }
      }
      internalSetQuery.call(this, p.currentQuery, p);
   };

   prototype.createQuery = function(field, comparison, value) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      switch (field) {
         case "Programme.startTime":
         case "Programme.endTime":
         case "Programme.name":
         case "Programme.programmeID":
            break;
         default:
            console.error("Unsupported field to compare: " + field);
            return null;
      }
      return hbbtv.objects.createQuery(field, comparison, value);
   };

   prototype.findProgrammesFromStream = function(channel, startTime, count) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      /* HbbTV 2.0.3: The count parameter is not included. */
      p.channelConstraints = [channel.ccid];
      if (!startTime) {
         startTime = Math.round(Date.now() / 1000);
      }
      const qEndTime = hbbtv.objects.createQuery("Programme.endTime", 2, startTime);
      internalSetQuery.call(this, qEndTime, p);
   };

   function initialise(searchManager, searchTarget) {
      privates.set(this, {});
      const p = privates.get(this);
      p.searchManager = (searchManager && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? new WeakRef(searchManager) : searchManager;
      p.searchTarget = searchTarget;
      p.internalState = State.IDLE;
      p.channelConstraints = [];
      p.result = hbbtv.objects.createSearchResults({
         empty: true
      }, this);

      p.onMetadataSearch = (event) => {
         console.log("Received MetadataSearch");
         console.log(event);

         if (p.currentQuery.queryId() !== event.search) {
            /* Ignore this search event */
            return;
         }
         dispatchMetadataSearch.call(this, event.search, event.status, event.programmeList, event.offset, event.totalSize);
      };
      hbbtv.bridge.addWeakEventListener("MetadataSearch", p.onMetadataSearch);
   }

   function internalSetQuery(query, p) {
      if (p.currentQuery) {
         if ((p.internalState != State.IDLE) && p.result) {
            p.result.abort();
         }
      }

      p.currentQuery = query;
      p.result = hbbtv.objects.createSearchResults({
         query: query
      }, this);
      p.internalState = State.IDLE;
   }

   function dispatchMetadataSearch(search, state, programmeList, offset, totalSize) {
      const p = privates.get(this);

      if (p.currentQuery.queryId() !== search) {
         /* Ignore this search event */
         return;
      }

      let handled = false;
      if (p.internalState == State.IDLE) {
         if (state === -1) {
            hbbtv.bridge.broadcast.startSearch(p.currentQuery, offset, totalSize, p.channelConstraints);
            handled = true;
            p.internalState = State.SEARCHING;
         }
      } else if (p.internalState == State.SEARCHING) {
         if (state === 0) {
            handled = true;
            p.internalState = State.FOUND;
         } else if ((state === 3) || (state === 4)) {
            handled = true;
            p.internalState = State.IDLE;
         }
      } else if (p.internalState == State.FOUND) {
         if (state === -1) {
            hbbtv.bridge.broadcast.startSearch(p.currentQuery, offset, totalSize, p.channelConstraints);
            handled = true;
            p.internalState = State.SEARCHING;
         } else if (state === 3) {
            handled = true;
            p.internalState = State.IDLE;
         }
      }
      if (!handled) {
         console.error("Received MetadataSearch state=" + state + " Event while " + p.internalState + ". programmeList=");
         console.error(programmeList);
         return;
      }
      if (programmeList) {
         p.result.setResults(programmeList, offset, totalSize);
      }
      if (state != -1) {
         /* Pass down the event to the searchManager */
         const searchManager = (p.searchManager && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? p.searchManager.deref() : p.searchManager;
         hbbtv.objects.OipfSearchManager.dispatchMetadataSearch.call(searchManager, this, state);
      }
   }

   /** Broadcast-independent applications: shall throw a "Security Error" */
   function mandatoryBroadcastRelatedSecurityCheck(p) {
      const bi = (p.searchManager && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? p.searchManager.deref() : p.searchManager;
      if (bi && bi.broadcastIndependent) {
         throw new DOMException('', 'SecurityError');
      }
   }

   return {
      prototype: prototype,
      initialise: initialise,
      dispatchMetadataSearch: dispatchMetadataSearch
   };
})();

hbbtv.objects.createMetadataSearch = function(searchManager, searchTarget) {
   const metadataSearch = Object.create(hbbtv.objects.MetadataSearch.prototype);
   hbbtv.objects.MetadataSearch.initialise.call(metadataSearch, searchManager, searchTarget);
   return metadataSearch;
};