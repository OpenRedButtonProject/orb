polyfill_mediamanager_srcs = \
src/mediamanager.js \
src/utils.js \
src/objects/audiotracklist.js \
src/objects/videotracklist.js \
src/mediaproxies/mediaerror.js \
src/mediaproxies/dashproxy.js \
src/mediaproxies/nativeproxy.js \
src/mediaproxies/iframe.js

polyfill_dash_srcs = \
external/dash.all.min.js

polyfill_hbbtv_srcs = \
src/housekeeping/banner.js \
src/housekeeping/beginiffe.js \
src/utils.js \
src/core.js \
src/nativemanager.js \
src/bridge.js \
src/holepuncher.js \
src/objectmanager.js \
src/natives/android.js \
src/natives/rdk.js \
src/polyfill/xhr.js \
src/polyfill/close.js \
src/polyfill/debug.js \
src/polyfill/keyevent.js \
src/polyfill/orbdebug.js \
src/objects/collection.js \
src/objects/audiotracklist.js \
src/objects/videotracklist.js \
src/objects/channel.js \
src/objects/programme.js \
src/objects/channellist.js \
src/objects/channelconfig.js \
src/objects/configuration/oipfconfiguration.js \
src/objects/configuration/configuration.js \
src/objects/configuration/localsystem.js \
src/objects/capabilities/oipfcapabilities.js \
src/objects/videobroadcast.js \
src/objects/avcomponent.js \
src/objects/avvideocomponent.js \
src/objects/avaudiocomponent.js \
src/objects/avsubtitlecomponent.js \
src/objects/keyset.js \
src/objects/privatedata.js \
src/objects/application.js \
src/objects/applicationmanager.js \
src/objects/avcontrol.js \
src/objects/parentalcontrol/oipfparentalcontrolmanager.js \
src/objects/parentalcontrol/parentalratingscheme.js \
src/objects/parentalcontrol/parentalratingschemecollection.js \
src/objects/parentalcontrol/parentalrating.js \
src/objects/parentalcontrol/parentalratingcollection.js \
src/objects/searchmanager/oipfsearchmanager.js \
src/objects/searchmanager/metadatasearch.js \
src/objects/searchmanager/query.js \
src/objects/searchmanager/searchresults.js \
src/objects/mediasync/mediasynchroniser.js \
src/objects/mediasync/mediaelementobserver.js \
src/objects/mediasync/broadcastobserver.js \
src/objects/mediasync/mediaelementtsclient.js \
src/objects/csmanager.js \
src/mediaproxies/mediaerror.js \
src/mediaproxies/mediaelementwrapper.js \
src/run.js \
src/housekeeping/endiffe.js

define build-polyfill
$(shell mkdir -p $(2); \
cat $(addprefix $(1)/,$(polyfill_dash_srcs)) > $(2)/dash.all.min.js; \
cat $(addprefix $(1)/,$(polyfill_hbbtv_srcs)) > $(2)/hbbtv.js; \
cat $(addprefix $(1)/,$(polyfill_mediamanager_srcs)) > $(2)/mediamanager.js)
endef

