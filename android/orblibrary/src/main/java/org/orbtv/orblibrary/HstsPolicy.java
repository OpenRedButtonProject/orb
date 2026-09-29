package org.orbtv.orblibrary;

import android.util.Log;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Known HSTS Hosts for the HbbTV user agent (RFC 6797). WebView never sees
 * these responses, because {@link WebResourceClient} performs the fetch, so
 * the policy has to be recorded here and applied before the next request.
 */
final class HstsPolicy {
    private static final String TAG = "HstsPolicy";
    private static final ConcurrentHashMap<String, Policy> POLICIES = new ConcurrentHashMap<>();

    private HstsPolicy() {
    }

    /**
     * Rewrite an http URL to https when a stored policy applies. Port 80 becomes
     * 443, any other explicit port is kept, and a URL with no port does not gain one.
     */
    static String upgrade(String url) {
        URI uri = parse(url);
        if (uri == null || !"http".equalsIgnoreCase(uri.getScheme()) || !applies(uri.getHost())) {
            return url;
        }
        String authority = uri.getRawAuthority();
        if (authority == null) {
            return url;
        }
        if (uri.getPort() == 80 && authority.endsWith(":80")) {
            authority = authority.substring(0, authority.length() - 3) + ":443";
        }
        StringBuilder rewritten = new StringBuilder("https://").append(authority);
        if (uri.getRawPath() != null) {
            rewritten.append(uri.getRawPath());
        }
        if (uri.getRawQuery() != null) {
            rewritten.append('?').append(uri.getRawQuery());
        }
        if (uri.getRawFragment() != null) {
            rewritten.append('#').append(uri.getRawFragment());
        }
        Log.i(TAG, "Upgrade " + url + " -> " + rewritten);
        return rewritten.toString();
    }

    /**
     * Store a policy from a Strict-Transport-Security header received on an
     * https response. An invalid header leaves any existing policy unchanged.
     * max-age=0 removes the host.
     */
    static void note(String requestUrl, List<String> headerValues) {
        URI uri = parse(requestUrl);
        if (uri == null || !"https".equalsIgnoreCase(uri.getScheme())) {
            return;
        }
        String host = normalize(uri.getHost());
        if (host == null || isIpAddress(host) || headerValues == null || headerValues.isEmpty()) {
            return;
        }
        Policy policy = parseHeader(headerValues.get(0));
        if (policy == null) {
            return;
        }
        if (policy.expiryMillis <= System.currentTimeMillis()) {
            POLICIES.remove(host);
            Log.i(TAG, "Removed Known HSTS Host " + host);
            return;
        }
        POLICIES.put(host, policy);
        Log.i(TAG, "Known HSTS Host " + host
                + (policy.includeSubDomains ? " includeSubDomains" : ""));
    }

    private static boolean applies(String host) {
        String normalized = normalize(host);
        if (normalized == null || isIpAddress(normalized)) {
            return false;
        }
        long now = System.currentTimeMillis();
        if (live(normalized, false, now)) {
            return true;
        }
        int from = 0;
        while (true) {
            int dot = normalized.indexOf('.', from);
            if (dot < 0 || dot + 1 >= normalized.length()) {
                return false;
            }
            if (live(normalized.substring(dot + 1), true, now)) {
                return true;
            }
            from = dot + 1;
        }
    }

    private static boolean live(String host, boolean requireIncludeSubDomains, long now) {
        Policy policy = POLICIES.get(host);
        if (policy == null) {
            return false;
        }
        if (policy.expiryMillis <= now) {
            POLICIES.remove(host, policy);
            return false;
        }
        return !requireIncludeSubDomains || policy.includeSubDomains;
    }

    private static Policy parseHeader(String field) {
        if (field == null) {
            return null;
        }
        Long maxAge = null;
        boolean includeSubDomains = false;
        boolean sawMaxAge = false;
        for (String directive : field.split(";")) {
            String trimmed = directive.trim();
            if (trimmed.isEmpty()) {
                continue;
            }
            String name = trimmed;
            String value = null;
            int equals = trimmed.indexOf('=');
            if (equals >= 0) {
                name = trimmed.substring(0, equals).trim();
                value = unquote(trimmed.substring(equals + 1).trim());
            }
            if (name.equalsIgnoreCase("max-age")) {
                if (sawMaxAge) {
                    continue;
                }
                sawMaxAge = true;
                maxAge = parseDeltaSeconds(value);
                if (maxAge == null) {
                    return null;
                }
            } else if (name.equalsIgnoreCase("includeSubDomains")) {
                includeSubDomains = true;
            }
        }
        if (maxAge == null) {
            return null;
        }
        long now = System.currentTimeMillis();
        long expiry;
        if (maxAge == 0 || maxAge > (Long.MAX_VALUE - now) / 1000L) {
            expiry = maxAge == 0 ? now : Long.MAX_VALUE;
        } else {
            expiry = now + maxAge * 1000L;
        }
        return new Policy(includeSubDomains, expiry);
    }

    private static Long parseDeltaSeconds(String value) {
        if (value == null || value.isEmpty() || value.length() > 18) {
            return null;
        }
        for (int i = 0; i < value.length(); i++) {
            if (!Character.isDigit(value.charAt(i))) {
                return null;
            }
        }
        try {
            return Long.parseLong(value);
        } catch (NumberFormatException e) {
            return null;
        }
    }

    private static String unquote(String value) {
        if (value.length() >= 2 && value.charAt(0) == '"' && value.charAt(value.length() - 1) == '"') {
            return value.substring(1, value.length() - 1);
        }
        return value;
    }

    private static String normalize(String host) {
        if (host == null) {
            return null;
        }
        String normalized = host.toLowerCase(Locale.US);
        while (normalized.endsWith(".")) {
            normalized = normalized.substring(0, normalized.length() - 1);
        }
        return normalized.isEmpty() ? null : normalized;
    }

    private static boolean isIpAddress(String host) {
        if (host.indexOf(':') >= 0) {
            return true;
        }
        String[] labels = host.split("\\.", -1);
        if (labels.length != 4) {
            return false;
        }
        for (String label : labels) {
            if (label.isEmpty() || label.length() > 3) {
                return false;
            }
            int value = 0;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                if (c < '0' || c > '9') {
                    return false;
                }
                value = value * 10 + (c - '0');
            }
            if (value > 255) {
                return false;
            }
        }
        return true;
    }

    private static URI parse(String url) {
        try {
            return new URI(url);
        } catch (URISyntaxException e) {
            return null;
        }
    }

    private static final class Policy {
        final boolean includeSubDomains;
        final long expiryMillis;

        Policy(boolean includeSubDomains, long expiryMillis) {
            this.includeSubDomains = includeSubDomains;
            this.expiryMillis = expiryMillis;
        }
    }
}
