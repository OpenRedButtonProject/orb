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
