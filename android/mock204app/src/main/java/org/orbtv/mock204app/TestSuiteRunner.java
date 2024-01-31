package org.orbtv.mock204app;

import android.content.Context;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.Vector;

public class TestSuiteRunner {
    private static final String TAG = TestSuiteRunner.class.getSimpleName();
    private final Context mContext;
    private final String mLocalHost;
    private final Callback mCallback;
    private final Vector<TestSuite> mTestSuites;
    private int mTestSuiteIndex = -1;

    interface Callback {
        void onTestSuiteStarted(String name, TestSuiteScenario scenario, String dsmccData, String action);

        void onTestSuiteFinished(String name, String report);

        void onFinished();
    }

    TestSuiteRunner(Context context, String fileName, String localHost, Callback callback)
            throws Exception {
        assert (callback != null);
        mContext = context;
        mLocalHost = localHost;
        mCallback = callback;
        JSONObject manifest = new JSONObject(new String(Utils.getAssetContents(context,
                "tests/" + fileName)));
        JSONArray testSuites = manifest.getJSONArray("testSuites");
        mTestSuites = new Vector<>(testSuites.length());
        for (int i = 0; i < testSuites.length(); i++) {
            JSONObject info = testSuites.getJSONObject(i);
            TestSuite testSuite = new TestSuite();
            testSuite.name = info.getString("name");
            testSuite.scenario = info.getString("scenario");
            testSuite.dsmccData = info.getString("dsmccData");
            testSuite.action = info.getString("action");
            testSuite.timeout = info.getInt("timeout");
            mTestSuites.add(testSuite);
        }
    }

    public void run() {
        if (mTestSuiteIndex == -1) {
            next();
        }
    }

    public void onTestSuiteError(String error) {
        if (mTestSuiteIndex < mTestSuites.size()) {
            TestSuite testSuite = mTestSuites.get(mTestSuiteIndex);
            mCallback.onTestSuiteFinished(testSuite.name, error); // TODO Wrap error in XML
            next();
        }
    }

    public void onTestReportPublished(String testSuiteName, String testReport) {
        if (mTestSuiteIndex < mTestSuites.size() &&
                mTestSuites.get(mTestSuiteIndex).name.equals(testSuiteName)) {
            mCallback.onTestSuiteFinished(testSuiteName, testReport);
            next();
        } else {
            Log.e(TAG, "Unexpected test suite name '" + testSuiteName + "'.");
        }
    }

    private void next() {
        if (++mTestSuiteIndex >= mTestSuites.size()) {
            mCallback.onFinished();
            return;
        }
        TestSuite testSuite = mTestSuites.get(mTestSuiteIndex);
        TestSuiteScenario scenario;
        try {
            scenario = new TestSuiteScenario(mContext, testSuite.scenario, mLocalHost);
        } catch (Exception e) {
            e.printStackTrace();
            onTestSuiteError("Failed to parse scenario.");
            return;
        }
        mCallback.onTestSuiteStarted(testSuite.name, scenario, testSuite.dsmccData, testSuite.action);
    }

    private static class TestSuite {
        public String name;
        public String scenario;
        public String dsmccData;
        public String action;
        public int timeout;
    }
}
