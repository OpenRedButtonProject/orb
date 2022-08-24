/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

import java.nio.ByteBuffer;

public interface IDsmccClient {

    /**
     * Called by DsmccEngine on receiving content
     *
     * @param requestId ID of request
     * @param buffer ByteBuffer with content for DSMCC file
     */
    void onReceiveContent(int requestId, ByteBuffer buffer);

    /**
     * Called by DsmccEngine on receiving Stream Event
     *
     * @param listenId ID of listener
     * @param name Name of Stream event
     * @param data Data asssociated with stream event
     */
    void onReceiveStreamEvent(int listenId, String name, String data, String text, String status);

}

