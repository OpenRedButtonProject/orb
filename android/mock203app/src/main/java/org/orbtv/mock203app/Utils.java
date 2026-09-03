/**
 * ORB Software. Copyright (c) 2026 Ocean Blue Software Limited
 *
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
 
package org.orbtv.mock203app;

import android.content.Context;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class Utils {
    public static byte[] getAssetContents(Context context, String fileName) throws IOException {
        byte[] buffer;
        InputStream inputStream = context.getAssets().open(fileName);
        buffer = new byte[inputStream.available()];
        inputStream.read(buffer);
        inputStream.close();
        return buffer;
    }

    public static void recursiveDelete(File file) {
        if (file.isDirectory()) {
            File[] files = file.listFiles();
            if (files != null) {
                for (File child : files) {
                    recursiveDelete(child);
                }
            }
        }
        file.delete();
    }
}
