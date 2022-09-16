package org.orbtv.tvbrowsershell;

import android.content.Context;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.orbtv.tvbrowser.TvBrowserTypes;

import java.util.Vector;

public class TestSuiteScenario {

   private final String mLocalHost;
   private final Vector<MockChannel> mMockChannels;
   private int mCurrentChannelIndex;
   private byte mVersionNumber = 0;

   public TestSuiteScenario(Context context, String fileName, String localHost) throws Exception {
      mLocalHost = localHost;
      mMockChannels = new Vector<>();
      JSONObject scenario = new JSONObject(new String(Utils.getAssetContents(context,
         "tests/" + fileName)));
      JSONArray channels = scenario.getJSONArray("channels");
      for (int i = 0; i < channels.length(); i++) {
         JSONObject info = channels.getJSONObject(i);
         MockChannel channel = new MockChannel(parseChannelFile(context,
            info.getString("channel")));
         channel.programmes = parseProgrammesFile(context, info.getString("programmes"));
         channel.components = parseComponentsFile(context, info.getString("components"));
         channel.applications = parseApplications(info.getJSONArray("applications"));
         mMockChannels.add(channel);
      }
      mCurrentChannelIndex = (mMockChannels.size() > 0) ? 0 : -1;
   }

   public boolean selectChannel(int onid, int tsid, int sid) {
      for (int i = 0; i < mMockChannels.size(); i++) {
         TvBrowserTypes.Channel channel = mMockChannels.get(i).channel;
         if (channel.onid == onid && channel.tsid == tsid && channel.sid == sid) {
            mCurrentChannelIndex = i;
            return true;
         }
      }
      return false;
   }

   public TvBrowserTypes.Channel getCurrentChannel() {
      if (mCurrentChannelIndex < 0) {
         return null;
      }
      return mMockChannels.get(mCurrentChannelIndex).channel;
   }

   public byte[] getCurrentChannelAit() {
      if (mCurrentChannelIndex < 0) {
         return null;
      }
      MockChannel ch = mMockChannels.get(mCurrentChannelIndex);
      // TODO Support more than 1 application
      if (ch.applications.size() < 1) {
         return null;
      }
      mVersionNumber = (byte) ((mVersionNumber + 1) % 0b11111);
      return new MockAit(ch.applications, mVersionNumber).toBytes();
   }

   public Vector<TvBrowserTypes.Channel> getMockChannels() {
      Vector<TvBrowserTypes.Channel> out = new Vector<>();
      for (MockChannel ch : mMockChannels) {
         out.add(ch.channel);
      }
      return out;
   }

   public Vector<TvBrowserTypes.Programme> getCurrentChannelProgrammes() {
      if (mCurrentChannelIndex < 0) {
         return null;
      }
      MockChannel ch = mMockChannels.get(mCurrentChannelIndex);
      return ch.programmes;
   }

   public Vector<TvBrowserTypes.Component> getCurrentChannelComponents() {
      if (mCurrentChannelIndex < 0) {
         return null;
      }
      MockChannel ch = mMockChannels.get(mCurrentChannelIndex);
      return ch.components;
   }

   private TvBrowserTypes.Channel parseChannelFile(Context context, String fileName)
         throws Exception {
      JSONObject info = new JSONObject(new String(Utils.getAssetContents(context,
         "tests/" + fileName)));
      return new TvBrowserTypes.Channel(
         true,
         createCcid(info.getInt("onid"), info.getInt("tsid"), info.getInt("sid")),
         info.getString("name"),
         null,
         info.getInt("channelType"),
         info.getInt("idType"),
         info.getInt("majorChannel"),
         info.getInt("terminalChannel"),
         info.getInt("nid"),
         info.getInt("onid"),
         info.getInt("tsid"),
         info.getInt("sid"),
         info.getBoolean("hidden"),
         -1,
         null
      );
   }

   private Vector<TvBrowserTypes.Programme> parseProgrammesFile(Context context, String fileName)
         throws Exception {
      Vector<TvBrowserTypes.Programme> programmes = new Vector<>();
      JSONArray array = new JSONArray(new String(Utils.getAssetContents(context,
         "tests/" + fileName)));
      for (int i = 0; i < array.length(); i++) {
         JSONObject info = array.getJSONObject(i);
         programmes.add(new TvBrowserTypes.Programme(
            info.getString("name"),
            info.getString("programmeId"),
            info.getInt("programmeIdType"),
            info.getString("description"),
            info.getString("longDescription"),
            info.getInt("startTime"),
            info.getInt("duration"),
            info.getString("channelId"),
            parseParentalRatings(context, info.getString("parentalRatings"))
         ));
      }
      return programmes;
   }

   private Vector<TvBrowserTypes.Component> parseComponentsFile(Context context, String fileName)
         throws Exception {
      Vector<TvBrowserTypes.Component> components = new Vector<>();
      JSONArray array = new JSONArray(new String(Utils.getAssetContents(context,
      "tests/" + fileName)));
      for (int i = 0; i < array.length(); i++) {
         JSONObject info = array.getJSONObject(i);
         switch (info.getString("type")) {
            case "video": {
               components.add(TvBrowserTypes.Component.createVideoComponent(
                  info.getInt("componentTag"),
                  info.getInt("pid"),
                  info.getString("encoding"),
                  info.getBoolean("encrypted"),
                  info.getString("aspectRatio").equals("16_9") ?
                     TvBrowserTypes.ASPECT_RATIO_16_9 :
                     TvBrowserTypes.ASPECT_RATIO_4_3,
                  info.getBoolean("active"),
                  info.getBoolean("defaultComponent")
               ));
               break;
            }
            case "audio": {
               TvBrowserTypes.Component component = TvBrowserTypes.Component.createAudioComponent(
                  info.getInt("componentTag"),
                  info.getInt("pid"),
                  info.getString("encoding"),
                  info.getBoolean("encrypted"),
                  info.getString("language"),
                  info.getBoolean("audioDescription"),
                  info.getInt("audioChannels"),
                  info.getBoolean("active"),
                  info.getBoolean("defaultComponent")
               );
               component.hidden = info.getBoolean("hidden"); // TODO Add to constructor
               components.add(component);
               break;
            }
            case "subtitle": {
               components.add(TvBrowserTypes.Component.createSubtitleComponent(
                  info.getInt("componentTag"),
                  info.getInt("pid"),
                  info.getString("encoding"),
                  info.getBoolean("encrypted"),
                  info.getString("language"),
                  info.getBoolean("hearingImpaired"),
                  info.getString("label"),
                  info.getBoolean("active"),
                  info.getBoolean("defaultComponent")
               ));
               break;
            }
            default: {
               throw new Exception("Unknown component type.");
            }
         }
      }

      return components;
   }

   private Vector<MockAit.Application> parseApplications(JSONArray applications)
         throws Exception {
      Vector<MockAit.Application> parsed = new Vector<>();
      for (int i = 0; i < applications.length(); i++) {
         JSONObject info = applications.getJSONObject(i);
         MockAit.Application application = new MockAit.Application();
         application.id = info.getInt("id");
         if (application.id >= Math.pow(2, 16)) {
            throw new Exception("Id out of bounds.");
         }
         application.orgId = info.getInt("orgId");
         if (application.orgId >= Math.pow(2, 32)) {
            throw new Exception("Organisation id out of bounds.");
         }
         application.name = info.getString("name");
         application.baseUrl = info.getString("baseUrl").replace("$LOCALHOST",
            mLocalHost);
         application.initialPath = info.getString("initialPath");
         parsed.add(application);
      }
      return parsed;
   }

   private Vector<TvBrowserTypes.ParentalRating> parseParentalRatings(Context context,
         String fileName) throws Exception {
      Vector<TvBrowserTypes.ParentalRating> ratings = new Vector<>();
      JSONArray array = new JSONArray(new String(Utils.getAssetContents(context,
         "tests/" + fileName)));
      for (int i = 0; i < array.length(); i++) {
         JSONObject info = array.getJSONObject(i);
         ratings.add(new TvBrowserTypes.ParentalRating(
            info.getString("name"),
            info.getString("scheme"),
            info.getInt("value"),
            info.getInt("labels"),
            info.getString("region")
         ));
      }
      return ratings;
   }

   private String createCcid(int onid, int tsid, int sid) {
      return String.format("dvb://%04x.%04x.%04x", onid, tsid, sid);
   }

   private static class MockChannel {
      MockChannel(TvBrowserTypes.Channel channel) {
         this.channel = channel;
      }
      public TvBrowserTypes.Channel channel;
      public Vector<TvBrowserTypes.Programme> programmes;
      public Vector<TvBrowserTypes.Component> components;
      public Vector<MockAit.Application> applications;
   }
}
