/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
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

package org.orbtv.mock204app;

import android.content.Context;
import android.util.Log;
import android.webkit.MimeTypeMap;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import okhttp3.mockwebserver.Dispatcher;
import okhttp3.mockwebserver.MockResponse;
import okhttp3.mockwebserver.MockWebServer;
import okhttp3.mockwebserver.RecordedRequest;

public class MockHttpServer {
    private final String TAG = MockHttpServer.class.getSimpleName();
    private final Context mContext;
    private final MockWebServer mServer;
    private String mLocalHost = "";

    MockHttpServer(Context context) {
        mContext = context;
        mServer = new MockWebServer();
        mServer.setDispatcher(new Dispatcher() {
            @Override
            public MockResponse dispatch(RecordedRequest recordedRequest) {
                String path = recordedRequest.getPath();
                Log.d(TAG, "Get path: " + path);
                okio.Buffer body = getAsset(path);
                if (body != null) {
                    MockResponse response = new MockResponse().setResponseCode(200);
                    response.addHeader("Content-Type", getMimeTypeFromUrl(path));
                    response.setBody(body);
                    return response;
                } else {
                    return new MockResponse().setResponseCode(404);
                }
            }
        });
        Thread thread = new Thread(() -> {
            try {
                mServer.start();
                mLocalHost = mServer.url("").toString().replaceAll("/$", "");
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        thread.start();
        try {
            thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public String getLocalHost() {
        return mLocalHost;
    }

    public void shutdown() {
        Thread thread = new Thread(() -> {
            try {
                mServer.shutdown();
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        thread.start();
        try {
            thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private okio.Buffer getAsset(String path) {
        try {
            okio.Buffer buffer = new okio.Buffer();
            InputStream is = mContext.getAssets().open("tests" + path);
            okio.Okio.buffer(okio.Okio.source(is)).readAll(buffer);
            return buffer;
        } catch (FileNotFoundException exception) {
            Log.d(TAG, "File not found: " + exception.getMessage());
            return null;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    private String getMimeTypeFromUrl(String url) {
        String type = "*/*";
        String extension = MimeTypeMap.getFileExtensionFromUrl(url);
        if (extension != null && !extension.equals("")) {
            String fromExtension = MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
            if (fromExtension != null) {
                type = fromExtension;
            }
        }
        return type;
    }
}
