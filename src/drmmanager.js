/**
 * @fileOverview DRM Manager.
 * @license ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.drmManager = (function() {
   const privates = new WeakMap();

   const Result = Object.freeze({
      STATUS_READY: 0,
      STATUS_UNKNOWN: 1,
      STATUS_INITIALISING: 2,
      STATUS_ERROR: 3,

      MSG_SUCCESSFUL: 0,
      MSG_UNKNOWN_ERROR: 1,
      MSG_CANNOT_PROCESS_REQUEST: 2,
      MSG_UNKNOWN_MIME_TYPE: 3,
      MSG_USER_CONSENT_NEEDED: 4,
      MSG_UNKNOWN_DRM_SYSTEM: 5,
      MSG_WRONG_FORMAT: 6,
   });
   const DRM_SYSTEM_CIPLUS = "CIPLUS";
   const CIPLUS_MSG_TYPE_SAS_CONNECT_REQUEST = "SAS_connect_rqst";
   const CIPLUS_MSG_TYPE_SAS_ASYNC_MSG = "SAS_async_msg";
   const CIPLUS_MSG_TYPE_SAS_CONNECT_REQUEST_TIMEOUT = 5000;
   const OIPF_APPLICATION_ID_v2_2 = "0108113101190000";
   const OIPF_APPLICATION_ID_v2_3 = "0108113101190001";
   const CIPLUS_OIPF_APPLICATION_IDS = [
      OIPF_APPLICATION_ID_v2_2,
      OIPF_APPLICATION_ID_v2_3,
   ];

   const CIPLUS_MIME_TYPE = "application/vnd.oipf.cspg-hexbinary";
   const DrmMimeTypes = Object.freeze([
      "application/vnd.marlin.drm.actiontoken+xml",
      "application/vnd.oipf.mippvcontrolmessage+xml",
      CIPLUS_MIME_TYPE,
      "application/vnd.marlin.drm.actiontoken2+xml"
   ]);

   let gMsgIdCounter = 0;
   let gCiplusTransactionId = 0;

   // Internal listeners
   function addBridgeEventListeners() {
      const p = privates.get(this);

      p.onDRMSystemStatusChange = (event) => {
         console.log("Received onDRMSystemStatusChange " + JSON.stringify(event));
         const p = privates.get(this);
         const prevCSPGCIPlusDiscovered = p.CSPGCIPlusDiscovered;
         console.log("prevCSPGCIPlusDiscovered=", prevCSPGCIPlusDiscovered);
         /* Contains DRMSystemIDs to notify with p.oipfDrmAgent.dispatchDRMSystemStatusChange */
         let DRMSystemStatusChanges;
         if (p.drmSystemMap.has(event.DRMSystem)) {
            const prevStatus = p.drmSystemMap.get(event.DRMSystem);
            if (event.status === Result.STATUS_UNKNOWN) {
               /* Delete from maps, and signal events */
               DRMSystemStatusChanges = prevStatus.DRMSystemIDs;
               for (const DRMSystemID of prevStatus.DRMSystemIDs) {
                  p.drmSystemIdStatusMap.delete(event.DRMSystemID);
               }
               p.drmSystemMap.delete(event.DRMSystem);
               if (event.DRMSystem === DRM_SYSTEM_CIPLUS) {
                  p.CSPGCIPlusDiscovered = false;
               }
               /* TODO: Delete messages from common queue */
            } else {
               DRMSystemStatusChanges = prevStatus.DRMSystemIDs
                  .filter(x => !event.DRMSystemIDs.includes(x))
                  .concat(event.DRMSystemIDs.filter(x => !prevStatus.DRMSystemIDs.includes(x)));

               /* Update status, and keep internal stuff */
               const prevStatusStatus = prevStatus.status;
               prevStatus.DRMSystemIDs = event.DRMSystemIDs;
               prevStatus.status = event.status;
               prevStatus.protectionGateways = event.protectionGateways;
               prevStatus.supportedFormats = event.supportedFormats;
               if (event.DRMSystem === DRM_SYSTEM_CIPLUS) {
                  p.CSPGCIPlusDiscovered = (event.status === Result.STATUS_READY) &&
                     (event.protectionGateways !== undefined) &&
                     event.protectionGateways.includes("ci+");
                  if ((event.status === Result.STATUS_READY) &&
                     (event.status !== prevStatusStatus) &&
                     (event.protectionGateways === undefined)) {
                     console.log("Initiating SAS_connect_rqst() #1");
                     tryNextAppId(p, null, prevStatus);
                  }
               }
            }
         } else if (event.status !== Result.STATUS_UNKNOWN) {
            DRMSystemStatusChanges = event.DRMSystemIDs;
            const status = {
               DRMSystem: event.DRMSystem,
               DRMSystemIDs: event.DRMSystemIDs,
               status: event.status,
               protectionGateways: event.protectionGateways,
               supportedFormats: event.supportedFormats,
            };
            p.drmSystemMap.set(event.DRMSystem, status);
            for (const DRMSystemID of event.DRMSystemIDs) {
               p.drmSystemIdStatusMap.set(DRMSystemID, status);
            }
            if (event.DRMSystem === DRM_SYSTEM_CIPLUS) {
               status.ciplusMessages = new Map();
               p.CSPGCIPlusDiscovered = (event.status === Result.STATUS_READY) &&
                  (event.protectionGateways !== undefined) &&
                  event.protectionGateways.includes("ci+");
               if ((event.status === Result.STATUS_READY) &&
                  (event.protectionGateways === undefined)) {
                  console.log("Initiating SAS_connect_rqst() #2");
                  tryNextAppId(p, null, status);
               }
            }
         }
         if (prevCSPGCIPlusDiscovered != p.CSPGCIPlusDiscovered) {

            if (p.oipfGatewayInfo) {
               console.log("Has oipfGatewayInfo, prevCSPGCIPlusDiscovered=" +
                  prevCSPGCIPlusDiscovered + " p.CSPGCIPlusDiscovered=" + p.CSPGCIPlusDiscovered);
               p.oipfGatewayInfo.dispatchDiscoverCSPGCIPlusEvent.call(p.oipfGatewayInfo.obj);
            }
         }
         console.log("DRMSystemStatusChanges=" + DRMSystemStatusChanges);
         if (p.oipfDrmAgent && DRMSystemStatusChanges) {
            for (const DRMSystemID of DRMSystemStatusChanges) {
               p.oipfDrmAgent.dispatchDRMSystemStatusChange.call(p.oipfDrmAgent.obj, DRMSystemID);
            }
         }
      };
      hbbtv.bridge.addWeakEventListener("DRMSystemStatusChange", p.onDRMSystemStatusChange);

      p.onDRMMessageResult = (event) => {
         console.log("Received onDRMMessageResult " + JSON.stringify(event));
         const p = privates.get(this);
         let raise = false;

         if (p.drmMessages.has(event.msgID)) {
            /* Message in the common queue */
            const msg = p.drmMessages.get(event.msgID);
            if (msg.timeout) {
               clearTimeout(msg.timeout);
               msg.timeout = null;
            }
            p.drmMessages.delete(event.msgID);
            if (p.drmSystemIdStatusMap.has(msg.DRMSystemID)) {
               const status = p.drmSystemIdStatusMap.get(msg.DRMSystemID);
               if (status.DRMSystem === DRM_SYSTEM_CIPLUS) {
                  /* If CI+, demux between normal messages and SAS_connect_rqst */
                  if (msg.msgType === CIPLUS_MSG_TYPE_SAS_CONNECT_REQUEST) {
                     console.log("Received SAS_connect_cnf status=" + JSON.stringify(event.resultMsg));
                     console.log(JSON.stringify(status));
                     if (event.resultMsg === "0") {
                        status.oipfAppId = msg.msg;
                        p.CSPGCIPlusDiscovered = true;
                        if (p.oipfGatewayInfo) {
                           p.oipfGatewayInfo.dispatchDiscoverCSPGCIPlusEvent.call(p.oipfGatewayInfo.obj);
                        }
                     } else {
                        console.log("SAS_connect_rqst for " + msg.msg + " failed status=" + event.resultMsg);
                        tryNextAppId(p, msg.msg, status);
                     }
                  } else {
                     console.error(DRM_SYSTEM_CIPLUS + " unhandled onDRMMessageResult");
                  }
               } else {
                  console.error("DRMSystemID " + msg.DRMSystemID + " unhandled");
               }
            } else {
               console.error("DRMSystemID " + msg.DRMSystemID + " not found");
            }
         } else {
            /* Custom DRM system message */
            console.error("msgId " + event.msgID + " not found");
            if (event.msgID === DRM_SYSTEM_CIPLUS) {
               console.error("CI+ specific message!");
               handleCiPlusMessage(p, event);
            }
         }

         if (p.oipfDrmAgent && raise) {
            p.oipfDrmAgent.dispatchDRMMessageResult.call(p.oipfDrmAgent.obj, event.msgID,
               event.resultMsg, event.resultCode);
         }
      };
      hbbtv.bridge.addWeakEventListener("DRMMessageResult", p.onDRMMessageResult);

      p.onDRMSystemMessage = (event) => {
         console.log("Received onDRMSystemMessage " + JSON.stringify(event));

         const p = privates.get(this);
         if (p.oipfDrmAgent) {
            if (p.drmSystemIdStatusMap.has(event.DRMSystemID)) {
               console.warn("Unhandled DRMSystemID " + event.DRMSystemID);
            }
         }
      };
      hbbtv.bridge.addWeakEventListener("DRMSystemMessage", p.onDRMSystemMessage);

      p.onDRMRightsError = (event) => {
         console.log("Received DRMRightsError " + JSON.stringify(event));
         p.videoBroadcast.dispatchDRMRightsError.call(p.videoBroadcast.obj, event.errorState,
            event.contentID, event.DRMSystemID, event.rightsIssuerURL);
      };
      hbbtv.bridge.addWeakEventListener("DRMRightsError", p.onDRMRightsError);
   }

   function initialise() {
      privates.set(this, {});
      const p = privates.get(this);

      p.CSPGCIPlusDiscovered = false;

      /* Associates msgId with Message data */
      p.drmMessages = new Map();
      /* Associates DRMSystem with status */
      p.drmSystemMap = new Map();
      /* Associates DRMSystemID with status */
      p.drmSystemIdStatusMap = new Map();
      const sysIds = hbbtv.bridge.drm.getSupportedDRMSystemIDs();
      if (sysIds.length != 0) {
         console.log("Current DRM System IDs:");
         console.log(JSON.stringify(sysIds));
         for (const status of sysIds) {
            if (status.DRMSystem === DRM_SYSTEM_CIPLUS) {
               /* Associates transaction_id with Message data */
               status.ciplusMessages = new Map();
               p.drmSystemMap.set(status.DRMSystem, status);
               for (const DRMSystemID of status.DRMSystemIDs) {
                  console.log("Associating " + DRMSystemID + " to " + status.DRMSystem);
                  p.drmSystemIdStatusMap.set(DRMSystemID, status);
               }
               if (status.protectionGateways !== undefined) {
                  if (status.protectionGateways.includes("ci+")) {
                     console.log("SAS_connect_rqst() SUCCESSFUL");
                     p.CSPGCIPlusDiscovered = true;
                  } else {
                     console.log("SAS_connect_rqst() FAILED");
                  }
               } else {
                  console.log("SAS_connect_rqst() hasn't been tried");
                  tryNextAppId(p, null, status);
               }
            } else {
               console.warn("Unhandled DRMSystem " + status.DRMSystem);
            }
         }
      }
      addBridgeEventListeners.call(this);
   }

   function registerOipfGatewayInfo(obj, dispatchDiscoverCSPGCIPlusEvent) {
      privates.get(this).oipfGatewayInfo = {
         obj: obj,
         dispatchDiscoverCSPGCIPlusEvent: dispatchDiscoverCSPGCIPlusEvent
      };
   }

   function registerOipfDrmAgent(obj, dispatchDRMSystemStatusChange, dispatchDRMMessageResult,
      dispatchDRMSystemMessage) {
      privates.get(this).oipfDrmAgent = {
         obj: obj,
         dispatchDRMSystemStatusChange: dispatchDRMSystemStatusChange,
         dispatchDRMMessageResult: dispatchDRMMessageResult,
         dispatchDRMSystemMessage: dispatchDRMSystemMessage
      };
   }

   function registerVideoBroadcast(obj, dispatchParentalRatingChange, dispatchParentalRatingError,
      dispatchDRMRightsError) {
      privates.get(this).videoBroadcast = {
         obj: obj,
         dispatchParentalRatingChange: dispatchParentalRatingChange,
         dispatchParentalRatingError: dispatchParentalRatingError,
         dispatchDRMRightsError: dispatchDRMRightsError
      };
   }

   function isCSPGCIPlusDiscovered() {
      return (privates.get(this).CSPGCIPlusDiscovered);
   }

   function getCSPGCIPlusStatus() {
      const p = privates.get(this);
      if (p.drmSystemMap.has(DRM_SYSTEM_CIPLUS)) {
         return p.drmSystemMap.get(DRM_SYSTEM_CIPLUS);
      }
      return null;
   }

   function sendDRMMessage(msgType, msg, DRMSystemID) {
      const p = privates.get(this);
      gMsgIdCounter++;
      const msgId = gMsgIdCounter.toString();
      console.log("sendDRMMessage msgId=" + msgId + " type=" + msgType);
      let result = null;
      if (!DrmMimeTypes.includes(msgType)) {
         result = Result.MSG_UNKNOWN_MIME_TYPE;
      } else if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
         const status = p.drmSystemIdStatusMap.get(DRMSystemID);
         if (status.DRMSystem === DRM_SYSTEM_CIPLUS) {
            if (msgType === CIPLUS_MIME_TYPE) {
               const base64Data = serializeCiplusMessage(DRMSystemID, 0x01, 0x01, msg);
               commonSendDRMMessage(status.ciplusMessages, "ci+" + gCiplusTransactionId, msgId,
                  CIPLUS_MSG_TYPE_SAS_ASYNC_MSG, base64Data, DRMSystemID);
            } else {
               result = Result.MSG_UNKNOWN_MIME_TYPE;
            }
         } else {
            console.warn("Unhandled DRMSystemID " + DRMSystemID);
         }
      } else {
         result = Result.MSG_UNKNOWN_DRM_SYSTEM;
      }
      if (result) {
         Promise.resolve().then(() => p.oipfDrmAgent.dispatchDRMMessageResult.call(
            p.oipfDrmAgent.obj, msgId, null, result));
      }
      return msgId;
   }

   function canPlayContent(DRMPrivateData, DRMSystemID) {
      const p = privates.get(this);
      if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
         const status = p.drmSystemIdStatusMap.get(DRMSystemID);
         if (status.DRMSystem === DRM_SYSTEM_CIPLUS) {
            console.log("CI+ oipfAppId=" + status.oipfAppId);
            if (status.oipfAppId === OIPF_APPLICATION_ID_v2_3) {
               const base64Data = serializeCiplusMessage(DRMSystemID, 0x06, 0x09, DRMPrivateData);
               const response = hbbtv.bridge.drm.sendDRMMessage("ci+" + gCiplusTransactionId,
                  CIPLUS_MSG_TYPE_SAS_ASYNC_MSG, base64Data, DRMSystemID, true);
               console.log("Response is " + JSON.stringify(response));
               return handleCiPlusMessage(response);
            }
         } else {
            console.warn("Unhandled DRMSystemID " + DRMSystemID);
         }
      }
      return false;
   }

   function canRecordContent(DRMPrivateData, DRMSystemID) {
      const p = privates.get(this);
      if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
         const status = p.drmSystemIdStatusMap.get(DRMSystemID);
         if (status.DRMSystem === DRM_SYSTEM_CIPLUS) {
            console.log("CI+ oipfAppId=" + status.oipfAppId);
            if (status.oipfAppId === OIPF_APPLICATION_ID_v2_3) {
               const base64Data = serializeCiplusMessage(DRMSystemID, 0x08, 0x09, DRMPrivateData);
               const response = hbbtv.bridge.drm.sendDRMMessage("ci+" + gCiplusTransactionId,
                  CIPLUS_MSG_TYPE_SAS_ASYNC_MSG, base64Data, DRMSystemID, true);
               console.log("Response is " + JSON.stringify(response));
               return handleCiPlusMessage(response);
            }
         } else {
            console.warn("Unhandled DRMSystemID " + DRMSystemID);
         }
      }
      return false;
   }

   function getDRMSystemStatus(DRMSystemID) {
      const p = privates.get(this);
      if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
         return p.drmSystemIdStatusMap.get(DRMSystemID).status;
      }
      return Result.STATUS_UNKNOWN;
   }

   function setActiveDRM() {
      /* Note: Not sure this needs to go into the bridge */
      return hbbtv.bridge.drm.setActiveDRM(DRMSystemID);
   }

   /* msgId2 is used to know the real msgId corresponding to an internal DRM system msgId */
   function commonSendDRMMessage(queue, msgId, msgId2, msgType, msg, DRMSystemID) {
      const queue_msg = {
         msgId: msgId,
         msgId2: msgId2,
         msgType: msgType,
         msg: msg,
         DRMSystemID: DRMSystemID
      };
      queue.set(msgId, queue_msg);

      hbbtv.bridge.drm.sendDRMMessage(msgId, msgType, msg, DRMSystemID, false);
      return queue_msg;
   }

   function tryNextAppId(p, msg, status) {
      /* Check lower ID if possible */
      let index = (msg === null) ? CIPLUS_OIPF_APPLICATION_IDS.length : CIPLUS_OIPF_APPLICATION_IDS.indexOf(msg);
      if ((index != -1) && (index > 0)) {
         index--;
         gMsgIdCounter++;
         /* Try next private_host_application_ID */
         console.log("SAS_connect_rqst(id=" + gMsgIdCounter.toString() + ") for " + CIPLUS_OIPF_APPLICATION_IDS[index]);
         const msg = commonSendDRMMessage(p.drmMessages, gMsgIdCounter.toString(), null,
            CIPLUS_MSG_TYPE_SAS_CONNECT_REQUEST, CIPLUS_OIPF_APPLICATION_IDS[index],
            status.DRMSystemIDs[0]);
         msg.timeout = setTimeout(function() {
            p.drmMessages.delete(msg.msgId);
            if (p.drmSystemIdStatusMap.has(msg.DRMSystemID)) {
               /* DRM System hasn't been removed */
               tryNextAppId(p, msg.msg, status);
            }
         }, CIPLUS_MSG_TYPE_SAS_CONNECT_REQUEST_TIMEOUT, msg, p, status);
      } else {
         console.log("No more AppIds for SAS_connect_rqst");
      }
   }

   function extractDataTypes(decodedData) {
      const send_datatype_nbr = decodedData.charCodeAt(7);
      let index = 8;
      console.log("send_datatype_nbr=0x" + send_datatype_nbr.toString(16));
      let dataTypes = new Map();
      for (let i = 0; i < send_datatype_nbr; i++) {
         const datatype_id = decodedData.charCodeAt(index);
         console.log("datatype_id=0x" + datatype_id.toString(16));
         const datatype_length = (decodedData.charCodeAt(index + 1) << 8) + decodedData.charCodeAt(index + 2);
         console.log("datatype_length=" + datatype_length);
         index += 3;

         let datatype = null;
         switch (datatype_id) {
            case 0x01:
               console.log("oipf_ca_vendor_specific_information");
               datatype = "";
               for (let x = 0; x < datatype_length; x++) {
                  datatype += ("0" + decodedData.charCodeAt(x + index).toString(16)).slice(-2);
               }
               break;
            case 0x02:
               console.log("oipf_country_code");
               datatype = [decodedData.charCodeAt(index), decodedData.charCodeAt(index + 1)];
               break;
            case 0x03:
               console.log("oipf_parental_control_url");
               if (datatype_length != 0) {
                  datatype = decodedData.substring(index, index + datatype_length);
               }
               break;
            case 0x04:
               console.log("oipf_rating_type");
               datatype = decodedData.charCodeAt(index);
               break;
            case 0x05:
               console.log("oipf_rating_value");
               datatype = decodedData.charCodeAt(index);
               break;
            case 0x06:
               console.log("oipf_rights_issuer_url");
               if (datatype_length != 0) {
                  datatype = decodedData.substring(index, index + datatype_length);
               }
               break;
            case 0x07:
               console.log("oipf_access_status");
               datatype = decodedData.charCodeAt(index);
               break;
            case 0x08:
               console.log("oipf_status");
               datatype = decodedData.charCodeAt(index);
               break;
               /*case 0x09:
                  console.log("oipf_drm_private_data");
                  break;
               case 0x0a:
                  console.log("oipf_can_play_status");
                  break;
               case 0x0b:
                  console.log("oipf_can_record_status");
                  break;*/
            default:
               if (datatype_id >= 0x0c) {
                  if (datatype_id < 0x80) {
                     console.log("Reserved datatype_id 0x" + command_id.toString(16));
                  } else {
                     console.log("User defined datatype_id 0x" + command_id.toString(16));
                  }
               } else {
                  console.log("Unhandled datatype_id 0x" + command_id.toString(16));
               }
         }
         console.log("datatype=" + datatype);

         const value = dataTypes.get(datatype_id);
         if (value) {
            value.push(datatype);
         } else {
            dataTypes.set(datatype_id, [datatype]);
         }

         index += datatype_length;
      }
      return dataTypes;
   }

   function handleCiPlusMessage(p, event) {
      const status = p.drmSystemMap.get(DRM_SYSTEM_CIPLUS);
      if (!status) return; /* Safeguard */
      console.log("Inside handleCiPlusMessage. Parsing message.");
      const decodedData = atob(event.resultMsg);
      console.log("decodedData.length=" + decodedData.length);
      const command_id = decodedData.charCodeAt(0);
      const ca_system_id = "urn:dvb:casystemid:" + ((decodedData.charCodeAt(1) << 8) + (decodedData.charCodeAt(2)));
      const transaction_id = "ci+" + ((decodedData.charCodeAt(3) << 24) + (decodedData.charCodeAt(4) << 16) + (decodedData.charCodeAt(5) << 8) + decodedData.charCodeAt(6));
      console.log("command_id=0x" + command_id.toString(16));
      console.log("ca_system_id=" + ca_system_id);
      console.log("transaction_id=" + transaction_id.toString(16));


      switch (command_id) {
         /*case 0x01: console.log("send_msg");
            break;
         case 0x06: console.log("can_play_content_req");
            break;
         case 0x08: console.log("can_record_content_req");
            break;*/
         case 0x02:
            console.log("reply_msg for transaction_id=" + transaction_id);
            const replyMsg = status.ciplusMessages.get(transaction_id);
            if (replyMsg) {
               console.log("CI+ Message found " + transaction_id);
               if (p.oipfDrmAgent) {
                  const dataTypes = extractDataTypes(decodedData);
                  const oipf_status = dataTypes.get(0x08);
                  if (oipf_status) {
                     const oipf_ca_vendor_specific_information = dataTypes.get(0x01);
                     let oipf_status_fixed = oipf_status[0];
                     if ((oipf_status_fixed < 0) || (oipf_status_fixed > 5)) {
                        oipf_status_fixed = 1;
                     } else if (oipf_status_fixed === 3) {
                        oipf_status_fixed = 6;
                     }
                     p.oipfDrmAgent.dispatchDRMMessageResult.call(p.oipfDrmAgent.obj,
                        replyMsg.msgId2, oipf_ca_vendor_specific_information[0], oipf_status_fixed);
                  }
               }
               status.ciplusMessages.delete(transaction_id);
            } else {
               console.log("CI+: Unknown transaction_id: " + transaction_id);
            }
            break;
         case 0x03:
            console.log("parental_control_info");
            if (p.videoBroadcast) {
               const dataTypes = extractDataTypes(decodedData);
               const oipf_access_status = dataTypes.get(0x07);
               const oipf_rating_type = dataTypes.get(0x04);
               const oipf_rating_value = dataTypes.get(0x05);
               const oipf_country_code = dataTypes.get(0x02);
               if (oipf_access_status && oipf_rating_type && oipf_rating_value) {
                  const ratings = [{
                     value: oipf_rating_value[0],
                     labels: 0,
                     region: String.fromCharCode(oipf_country_code[0][0], oipf_country_code[0][1]),
                  }];
                  if (oipf_rating_type[0] === 0) {
                     ratings[0].scheme = "dvb-si";
                     ratings[0].name = oipf_rating_value[0].toString();
                     p.videoBroadcast.dispatchParentalRatingChange.call(p.videoBroadcast.obj, null,
                        ratings, ca_system_id, oipf_access_status[0] === 0 /* blocked */ );
                  } else {
                     p.videoBroadcast.dispatchParentalRatingError.call(p.videoBroadcast.obj, null,
                        ratings, ca_system_id);
                  }
               }
            }
            break;
         case 0x04:
            console.log("rights_info");
            if (p.videoBroadcast) {
               const dataTypes = extractDataTypes(decodedData);
               const oipf_access_status = dataTypes.get(0x07);
               const oipf_rights_issuer_url = dataTypes.get(0x06);
               if (oipf_access_status && oipf_rights_issuer_url) {
                  const errorState = (oipf_access_status[0] === 1) ? 2 : 0;
                  p.videoBroadcast.dispatchDRMRightsError.call(p.videoBroadcast.obj, errorState,
                     null, ca_system_id, oipf_rights_issuer_url[0]);
               }
            }
            break;
         case 0x05:
            console.log("system_info");
            if (p.oipfDrmAgent) {
               const dataTypes = extractDataTypes(decodedData);
               const value = dataTypes.get(0x01);
               if (value) {
                  for (const dataType of value) {
                     p.oipfDrmAgent.dispatchDRMSystemMessage.call(p.oipfDrmAgent.obj,
                        dataType.datatype, ca_system_id);
                  }
               }
            }
            break;
         case 0x07:
            console.log("can_play_content_reply");
            const canPlayMsg = status.ciplusMessages.get(transaction_id);
            if (canPlayMsg) {
               console.log("CI+ Can Play Message found " + transaction_id);
               if (p.oipfDrmAgent) {
                  const dataTypes = extractDataTypes(decodedData);
                  const oipf_can_play_status = dataTypes.get(0x0a);
                  console.log("oipf_can_play_status=" + oipf_can_play_status);
                  if (oipf_can_play_status) {
                     return (oipf_can_play_status[0] === 1);
                  }
               }
               status.ciplusMessages.delete(transaction_id);
            } else {
               console.log("CI+: Unknown transaction_id: " + transaction_id);
            }
            return false;
         case 0x09:
            console.log("can_record_content_reply");
            const canRecordMsg = status.ciplusMessages.get(transaction_id);
            if (canRecordMsg) {
               console.log("CI+ Can Record Message found " + transaction_id);
               if (p.oipfDrmAgent) {
                  const dataTypes = extractDataTypes(decodedData);
                  const oipf_can_record_status = dataTypes.get(0x0b);
                  console.log("oipf_can_record_status=" + oipf_can_record_status);
                  if (oipf_can_record_status) {
                     return (oipf_can_record_status[0] === 1);
                  }
               }
               status.ciplusMessages.delete(transaction_id);
            } else {
               console.log("CI+: Unknown transaction_id: " + transaction_id);
            }
            return false;
         default:
            if (command_id >= 0x0a) {
               if (command_id < 0x80) {
                  console.log("Reserved command_id 0x" + command_id.toString(16));
               } else {
                  console.log("User defined command_id 0x" + command_id.toString(16));
               }
            } else {
               console.log("Unhandled command_id 0x" + command_id.toString(16));
            }
      }
   }

   function urnToInt(urn) {
      return parseInt(urn.substring(urn.lastIndexOf(":") + 1));
   }

   function serializeCiplusMessage(DRMSystemID, command_id, datatype_id, data) {
      const message_bytes = [];
      message_bytes.push(command_id); //command_id
      const iDRMSystemID = urnToInt(DRMSystemID);
      message_bytes.push((iDRMSystemID >> 8) & 0xff); //ca_system_id
      message_bytes.push(iDRMSystemID & 0xff);
      gCiplusTransactionId = (gCiplusTransactionId + 1) & 0xffffffff; //modulo 2**32
      message_bytes.push((gCiplusTransactionId >> 24) & 0xff); //transaction_id
      message_bytes.push((gCiplusTransactionId >> 16) & 0xff);
      message_bytes.push((gCiplusTransactionId >> 8) & 0xff);
      message_bytes.push(gCiplusTransactionId & 0xff);
      message_bytes.push(1); //send_datatype_nbr = 1
      message_bytes.push(datatype_id); //datatype_id
      const len = data.length / 2;
      message_bytes.push((len >> 8) & 0xff); //datatype_length
      message_bytes.push(len & 0xff);
      for (let i = 0; i < data.length; i += 2) {
         message_bytes.push(parseInt(data.substring(i, i + 2), 16));
      }
      return btoa(String.fromCharCode.apply(null, message_bytes));
   }

   return {
      initialise: initialise,
      /* oipfGatewayInfo */
      registerOipfGatewayInfo: registerOipfGatewayInfo,
      isCSPGCIPlusDiscovered: isCSPGCIPlusDiscovered,
      /* oipfCapabilities and oipfGatewayInfo */
      getCSPGCIPlusStatus: getCSPGCIPlusStatus,
      /* oipfDrmAgent */
      registerOipfDrmAgent: registerOipfDrmAgent,
      sendDRMMessage: sendDRMMessage,
      canPlayContent: canPlayContent,
      canRecordContent: canRecordContent,
      getDRMSystemStatus: getDRMSystemStatus,
      setActiveDRM: setActiveDRM,
      /* VideoBroadcast */
      registerVideoBroadcast: registerVideoBroadcast,
   };
})();