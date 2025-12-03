define build-polyfill

$(eval polyfill_dash_srcs = \
external/dash.all.min.js)

$(eval polyfill_hbbtv_srcs = \
src/housekeeping/banner.js \
src/housekeeping/beginiffe.js \
src/languagecodes.js \
src/utils.js \
src/core.js \
src/bridge.js \
src/holepuncher.js \
src/objectmanager.js \
src/drmmanager.js \
src/natives/$(3).js \
src/polyfill/xhr.js \
src/polyfill/close.js \
src/polyfill/debug.js \
src/polyfill/keyevent.js \
src/polyfill/orbdebug.js \
src/objects/collection.js \
src/mediaproxies/tracklists/audiotracklist.js \
src/mediaproxies/tracklists/videotracklist.js \
src/objects/channel.js \
src/objects/programme.js \
src/objects/dashevent.js \
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
src/objects/oipfgatewayinfo.js \
src/objects/oipfdrmagent.js \
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
src/objects/oipfdrmagent.js \
src/objects/oipfgatewayinfo.js \
src/extensions/textinputextension.js \
src/mediaproxies/mediaerror.js \
src/mediaproxies/mediamanager.js \
src/mediaproxies/dashproxy.js \
src/mediaproxies/nativeproxy.js)

$(eval polyfill_proprietary_bbc_srcs = \
src/proprietary/bbc/bbc.js)

$(shell mkdir -p $(2); \
cat $(addprefix $(1)/,$(polyfill_dash_srcs)) > $(2)/hbbtv.js; \
echo "" >> $(2)/hbbtv.js; \
cat $(addprefix $(1)/,$(polyfill_hbbtv_srcs)) >> $(2)/hbbtv.js; \
if [ $(4) -eq 1 ]; then cat $(addprefix $(1)/,$(polyfill_proprietary_bbc_srcs)) >> $(2)/hbbtv.js; fi; \
cat $(addprefix $(1)/,src/run.js) >> $(2)/hbbtv.js; \
cat $(addprefix $(1)/,src/housekeeping/endiffe.js) >> $(2)/hbbtv.js;)
endef
