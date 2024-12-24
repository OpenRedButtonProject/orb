/* IOrbcSession.aidl - interface to ORB C++ from DVB integration */

package org.orbtv.orbservice;

import org.orbtv.orbservice.IDvbiSession;
import org.orbtv.orbservice.DataBuffer;

interface IOrbcSession {

    void initialise(in IDvbiSession dvb);

    /**
     * Requests the HbbTV engine to process the specified AIT. The HbbTV engine expects the relevant
     * AITs only (the first one after HBBTV_Start and when the version/PID changes). If more than one
     * stream is signalled in the PMT for a service with an application_signalling_descriptor, then
     * the application_signalling_descriptor for the stream containing the AIT for the HbbTV
     * application shall include the HbbTV application_type (0x0010).
     *
     * @param aitPid    PID of the AIT
     * @param serviceId Service ID the AIT refers to
     * @param data      The buffer containing the AIT row data
     */
    void processAIT(int aitPid, int serviceId, in DataBuffer data);

    /**
     * Called when the service list has changed.
     */
    void onServiceListChanged();

    /**
     * Called when the parental rating of the currently playing service has changed.
     *
     * @param blocked TRUE if the current service is blocked by the parental control system.
     */
    void onParentalRatingChanged(boolean blocked);

}
