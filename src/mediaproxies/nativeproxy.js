/**
 * @fileOverview The proxy for native playback.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.NativeProxy = (function() {
    const prototype = Object.create(HTMLMediaElement.prototype);
    const privates = new WeakMap();

    prototype.getStartDate = function() {
        return new Date(NaN);
    };

    prototype.orb_getSource = function() {
        return this.src;
    };

    prototype.orb_getPeriods = function() {
        return undefined;
    };

    prototype.orb_getMrsUrl = function() {
        return undefined;
    };

    prototype.orb_getCurrentPeriod = function() {
        return undefined;
    };

    prototype.orb_getCiAncillaryData = function() {
        return undefined;
    };

    prototype.orb_invalidate = function() {
        const p = privates.get(this);
        if (p) {
            this.textTracks.removeEventListener('change', p.onTextTrackChange);
            this.removeEventListener('loadedmetadata', p.onLoadedMetadata, true);
            this.removeEventListener('error', p.onError, true);
            if (p.subs.parentNode) {
                p.subs.parentNode.removeChild(p.subs);
            }
            if (p.textTracks) {
                p.textTracks.deleteAllTextTracks();
            }
            console.log('NativeProxy: Cleaning up...');
            privates.delete(this);
        }
    };

    function onLoadedMetadata() {
        let promises = [];
        const thiz = this;
        const p = privates.get(this);
        const ttmlParser = p.ttmlParser;
        const trackElements = this.getElementsByTagName('track');
        const config = {
            videoModel: p.videoModel,
            streamInfo: {
                id: 0,
                start: 0,
                duration: this.duration,
            },
        };
        const textTracks = orb_dashjs.TextTracks(window).create(config);
        if (p.textTracks) {
            p.textTracks.deleteAllTextTracks();
        }
        p.textTracks = textTracks;
        textTracks.initialize();
        for (let i = 0; i < this.textTracks.length; ++i) {
            const track = this.textTracks[i];
            const trackElement = trackElements[i];
            if (!trackElement) {
                promises.push(Promise.reject("Text track doesn't match a track element."));
                break;
            }

            const textTrackInfo = new orb_dashjs.TextTrackInfo();
            textTrackInfo.isEmbedded = false;
            textTrackInfo.streamInfo = config.streamInfo;
            textTrackInfo.isTTML = true;
            textTrackInfo.captionData = null;
            textTrackInfo.kind = track.kind;
            textTrackInfo.id = track.id;
            textTrackInfo.lang = track.language;
            textTrackInfo.labels = track.label;
            textTrackInfo.isFragmented = false;

            const ext = trackElement.src.split('.').pop();
            if (ext === 'ttml' || ext === 'xml') {
                promises.push(
                    new Promise((resolve, reject) => {
                        const xhr = new XMLHttpRequest();
                        xhr.onreadystatechange = function() {
                            if (xhr.readyState != XMLHttpRequest.DONE) return;
                            if (xhr.status !== 0 && xhr.status != 200 && xhr.status != 304) {
                                reject(
                                    "An error occurred when requesting the ttml source file '" +
                                    trackElement.src +
                                    "'."
                                );
                                return;
                            }
                            textTrackInfo.defaultTrack = track.mode === 'showing';
                            textTrackInfo.captionData = ttmlParser.parse(xhr.responseText, 0);
                            textTracks.addTextTrack(textTrackInfo);

                            // we remove the track elements, because textTracks.addTextTrack()
                            // pushes a new text track to the video element's text tracks list,
                            // ending up having twice as many tracks as we should be having
                            thiz.removeChild(trackElement);
                            resolve();
                        };
                        xhr.open('GET', trackElement.src);
                        xhr.send();
                    })
                );
            }
        }
        Promise.all(promises)
            .then(() => {
                textTracks.createTracks();
                for (const track of this.textTracks) {
                    track.encoding = 'application/ttml+xml';
                }
                p.onTextTrackChange();
            })
            .catch((e) => {
                console.warn('NativeProxy: Failed to populate texttracks. Error:', e);
            });
    }

    function onError() {
        if (this.error) {
            let evt = new Event('__obs_onerror__');
            let data = {
                code: 2,
                message: this.error.message,
            };

            if (hbbtv.native.name === 'rdk') {
                switch (this.error.code) {
                    case 3: // This file contains no playable streams
                        data.code = 4;
                        break;
                    case 4: // Connection Timeout
                        data.code = 1;
                        break;
                    default:
                        data.code = 2;
                        break;
                }
            } else {
                // normally, errors should be the distinguished by checking the value of
                // videoElement.error.code, but since its value is mostly 4, the only way
                // to differentiate the error events is by the videoElement.error.message value
                const err = this.error.message.split(':')[0];
                switch (err) {
                    case 'MEDIA_ELEMENT_ERROR':
                        data.code = 1; // MEDIA_ERR_NETWORK
                        break;
                    case 'DEMUXER_ERROR_COULD_NOT_OPEN':
                        data.code = 4; // MEDIA_ERR_DECODE
                        break;
                    case 'DEMUXER_ERROR_NO_SUPPORTED_STREAMS':
                        data.code = 0; // MEDIA_ERR_SRC_NOT_SUPPORTED
                        break;
                    default:
                        data.code = 2; // Unidentified error
                        break;
                }
            }

            Object.assign(evt, {
                error: data,
            });

            this.dispatchEvent(evt);
        }
    }

    function onTextTrackChange() {
        const p = privates.get(this);
        let index = -1;
        for (let i = 0; i < this.textTracks.length; ++i) {
            if (this.textTracks[i].mode === 'showing') {
                index = i;
                break;
            }
        }
        if (index !== -1) {
            if (this.parentNode && !p.subs.parentNode) {
                if (this.nextSibling) {
                    this.parentNode.insertBefore(p.subs, this.nextSibling);
                } else {
                    this.parentNode.appendChild(p.subs);
                }
            }
        } else if (p.subs.parentNode) {
            p.subs.parentNode.removeChild(p.subs);
        }
        if (p.textTracks) {
            p.textTracks.setCurrentTrackIdx(index);
        }
    }

    function initialise(src) {
        if (!privates.get(this)) {
            if (this.orb_invalidate) {
                this.orb_invalidate();
            }
            Object.setPrototypeOf(this, prototype);
            privates.set(this, {});
            const p = privates.get(this);
            const subtitlesRenderDiv = document.createElement('div');
            subtitlesRenderDiv.id = '__obs_subsPH__';
            p.subs = subtitlesRenderDiv;
            p.onTextTrackChange = onTextTrackChange.bind(this);
            p.onLoadedMetadata = onLoadedMetadata.bind(this);
            p.onError = onError.bind(this);
            p.videoModel = orb_dashjs.VideoModel(window).getInstance();
            p.videoModel.setElement(this);
            p.videoModel.setTTMLRenderingDiv(subtitlesRenderDiv);
            p.ttmlParser = orb_dashjs.TTMLParser(window).getInstance();

            this.textTracks.addEventListener('change', p.onTextTrackChange);
            this.addEventListener('loadedmetadata', p.onLoadedMetadata, true);
            this.addEventListener('error', p.onError, true);

            console.log('NativeProxy: Initialised NativeProxy.');
        } else {
            console.log('NativeProxy: Already initialised. Skipping...');
        }
    }

    return {
        initialise: initialise,
    };
})();

hbbtv.mediaManager.registerObject({
    initialise: function(object, src) {
        hbbtv.objects.NativeProxy.initialise.call(object, src);
    },
    onSourceAboutToChange: function(object, src) {},
    getName: function() {
        return 'native';
    },
    // TODO: add more extensions
    getSupportedExtensions: function() {
        return ['mp4', 'mp3', 'wav'];
    },
    // TODO: add more content types
    getSupportedContentTypes: function() {
        return ['video/mp4', 'audio/mp4', 'audio/mpeg'];
    },
});