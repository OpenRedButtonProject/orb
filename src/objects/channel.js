/**
 * @fileOverview OIPF Channel class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#channel-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.Channel = (function() {
   const prototype = {};
   const privates = new WeakMap();

   hbbtv.utils.defineConstantProperties(prototype, {
      TYPE_TV: 0,
      TYPE_RADIO: 1,
      TYPE_OTHER: 2,
      TYPE_ALL: 128,
      TYPE_HBBTV_DATA: 256,
      ID_ANALOG: 0,
      ID_DVB_C: 10,
      ID_DVB_S: 11,
      ID_DVB_T: 12,
      ID_DVB_SI_DIRECT: 13,
      ID_DVB_C2: 14,
      ID_DVB_S2: 15,
      ID_DVB_T2: 16,
      ID_ISDB_C: 20,
      ID_ISDB_S: 21,
      ID_ISDB_T: 22,
      ID_ATSC_T: 30,
      ID_IPTV_SDS: 40,
      ID_IPTV_URI: 41
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {ChannelType}
    *
    * @name channelType
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "channelType", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.channelType;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {CCid}
    *
    * @name ccid
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "ccid", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.ccid;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Dsd}
    *
    * @name dsd
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "dsd", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.dsd;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IdType}
    *
    * @name idType
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "idType", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.idType;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {NId}
    *
    * @name nId
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "nid", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.nid;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {OnId}
    *
    * @name onId
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "onid", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.onid;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {TsId}
    *
    * @name tsId
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "tsid", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.tsid;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {SId}
    *
    * @name SId
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "sid", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.sid;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Name}
    *
    * @name name
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "name", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.name;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {MajorChannel}
    *
    * @name majorChannel
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "majorChannel", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.majorChannel;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {TerminalChannel}
    *
    * @name terminalChannel
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "terminalChannel", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.terminalChannel;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {SourceID}
    *
    * @name sourceID
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "sourceID", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.sourceID;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IpBroadcastID}
    *
    * @name ipBroadcastID
    * @memberof Channel#
    */
   Object.defineProperty(prototype, "ipBroadcastID", {
      get: function() {
         const p = privates.get(this);
         return p.channelData.ipBroadcastID;
      }
   });

   // Initialise an instance of prototype
   function initialise(channelData) {
      privates.set(this, {});
      const p = privates.get(this);
      p.channelData = channelData; // Hold reference to caller's object
   }

   // Private method to get a copy of the channel data
   function cloneChannelData() {
      return Object.assign({}, privates.get(this).channelData);
   }

   prototype.toString = function() {
      return JSON.stringify(privates.get(this).channelData);
   }

   return {
      prototype: prototype,
      initialise: initialise,
      cloneChannelData: cloneChannelData
   }
})();

hbbtv.objects.createChannel = function(channelData) {
   // Create new instance of hbbtv.objects.Channel.prototype
   const channel = Object.create(hbbtv.objects.Channel.prototype);
   hbbtv.objects.Channel.initialise.call(channel, channelData);
   return channel;
};