/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowsershell;

import java.util.Vector;

class MockAit {
    // A mock AIT builder

    public static class Application {
        public short id;
        public String name;
        public String baseUrl;
        public String initialPath;
    }

    private final Vector<Byte> bytes = new Vector<>();
    private int cursor = 0;

    public MockAit(Application application, byte versionNumber) {
        assert (versionNumber <= 0b11111);
        int stringsLength = application.name.length() + application.baseUrl.length() +
                application.initialPath.length();

        insert(0x74, 8); // table_id
        insert(0b1, 1); // section_syntax_indicator
        insert(0b1, 1); // reserved_future_use
        insert(0b11, 2); // reserved
        insert(48 + stringsLength, 12); // section_length (after this field)
        insert(0b0, 1); // test_application_flag
        insert(0x10, 15); // application_type
        insert(0b11, 2); // reserved
        insert(versionNumber, 5); // version_number
        insert(0b1, 1); // current_next_indicator
        insert(0, 8); // section_number
        insert(0, 8); // last_section_number
        insert(0b1111, 4); // reserved_future_use
        insert(0, 12); // common_descriptors_length
        insert(0b1111, 4); // reserved_future_use
        insert(35 + stringsLength, 12); // application_loop_length

        // applications
        insert(500, 32); // application_identifier/organisation_id
        insert(application.id, 16); // application_identifier/application_id
        insert(0x01, 8); // application_control_code
        insert(0b1111, 4); // reserved_future_use
        insert(26 + stringsLength, 12); // application_descriptors_loop_length

        // application descriptor
        insert(0x00, 8); // descriptor_tag
        insert(9, 8); // descriptor_length
        insert(5, 8); // application_profiles_length
        insert(0x0000, 16); // application_profile
        insert(1, 8); // version.major
        insert(1, 8); // version.minor
        insert(1, 8); // version.micro
        insert(0b0, 1); // service_bound_flag
        insert(0b11, 2); // visibility
        insert(0b11111, 5); // reserved_future_use
        insert(5, 8); // application_priority
        insert(0, 8); // transport_protocol_label[n]

        // application name descriptor
        insert(0x01, 8); // descriptor_tag
        insert(4 + application.name.length(), 8); // descriptor_length
        String langCode = "eng";
        for (char ch : langCode.toCharArray()) {
            insert(ch, 8);
        }
        insert(application.name.length(), 8); // application_name_length
        for (char ch : application.name.toCharArray()) {
            insert(ch, 8);
        }

        // transport protocol descriptor
        insert(0x02, 8); // descriptor_tag
        insert(5 + application.baseUrl.length(), 8); // descriptor_length
        insert(0x0003, 16); // protocol_id
        insert(0, 8); // transport_protocol_label
        insert(application.baseUrl.length(), 8); // URL_base_length
        for (char ch : application.baseUrl.toCharArray()) {
            insert(ch, 8);
        }
        insert(0, 8); // URL_extension_count

        // simple application location descriptor
        insert(0x15, 8); // descriptor_tag
        insert(application.initialPath.length(), 8); // descriptor_length
        for (char ch : application.initialPath.toCharArray()) {
            insert(ch, 8);
        }

        insert(0, 32); // CRC32 (dummy value as we don't check it at this level anyway)
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public byte[] toBytes() {
        byte[] out = new byte[bytes.size()];
        for (int i = 0; i < bytes.size(); i++) {
            out[i] = bytes.get(i);
        }
        return out;
    }

    private void insert(int bits, int nbits) {
        for (int i = nbits - 1; i >= 0; i--) {
            int index = cursor / 8;
            if (bytes.size() <= index) {
                bytes.add((byte) 0);
            }
            int bit = (((bits >> i) & 1) << (7 - (cursor % 8)));
            bytes.set(index, (byte) (bytes.get(index) | bit));
            cursor++;
        }
    }
}