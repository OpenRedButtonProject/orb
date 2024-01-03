/**
 * @fileOverview Configuration class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#configuration-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

hbbtv.objects.Configuration = (function() {
    const prototype = {};
    let gIsPendingAccessCallback = false;

    hbbtv.utils.defineGetterProperties(prototype, {
        preferredAudioLanguage: hbbtv.bridge.configuration.getPreferredAudioLanguage,
        preferredAudioLanguage47: hbbtv.bridge.configuration.getPreferredAudioLanguage,
        preferredSubtitleLanguage: hbbtv.bridge.configuration.getPreferredSubtitleLanguage,
        preferredUILanguage: hbbtv.bridge.configuration.getPreferredUILanguage,
        countryId: hbbtv.bridge.configuration.getCountryId,
        subtitlesEnabled: hbbtv.bridge.configuration.getSubtitlesEnabled,
        audioDescriptionEnabled: hbbtv.bridge.configuration.getAudioDescriptionEnabled,
        cleanAudioEnabled: hbbtv.bridge.configuration.getCleanAudioEnabled,
        timeShiftSynchronized() {
            return false; // PVR
        },
        dtt_network_ids: hbbtv.bridge.configuration.getDttNetworkIds,
        deviceId: hbbtv.bridge.configuration.getDeviceId,
    });

    prototype.requestAccessToDistinctiveIdentifier = function(callback) {
        const wrapper = (event) => {
            // One shot event
            hbbtv.bridge.removeStrongEventListener('accesstodistinctiveidentifier', wrapper);
            callback(event.allowAccess);
            gIsPendingAccessCallback = false;
        };
        hbbtv.bridge.addStrongEventListener('accesstodistinctiveidentifier', wrapper);
        if (!gIsPendingAccessCallback) {
            gIsPendingAccessCallback = true;
            hbbtv.bridge.configuration.requestAccessToDistinctiveIdentifier();
        }
    };

    function initialise() {}

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createConfiguration = function() {
    const configuration = Object.create(hbbtv.objects.Configuration.prototype);
    hbbtv.objects.Configuration.initialise.call(configuration);
    return configuration;
};
