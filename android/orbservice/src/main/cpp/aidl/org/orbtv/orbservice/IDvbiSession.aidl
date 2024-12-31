/* IDvbiSession.aidl - interface to DVB Integration from ORB C++ */

package org.orbtv.orbservice;

interface IDvbiSession {

    /**
     * Gets a string containing languages to be used for the UI, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    byte[] getPreferredUILanguage();

    /**
     * Gets a string containing the three character country code identifying the country in which the
     * receiver is deployed.
     *
     * @return Country code (ISO 3166 alpha-3) string
     */
    byte[] getCountryId();

    /**
     * Gets whether subtitles are enabled in the TV context. So HbbTV knows to start subtitle
     * components on channel change, for example.
     *
     * @return true if enabled, false otherwise
     */
    boolean getSubtitlesEnabled();

    /**
     * Gets whether audio description is enabled in the TV context.
     *
     * @return true if enabled, false otherwise
     */
    boolean getAudioDescriptionEnabled();

    /**
     * Returns the CCID of the current channel
     *
     * @return A CCID on success, an empty string otherwise
     */
   byte[] getCurrentCcid();

}
