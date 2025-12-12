#ifndef ORB_SRV_RECORD_H
#define ORB_SRV_RECORD_H

#include <string>
#include <cstdint>

namespace orb {

/**
 * @brief Represents a DNS SRV record (RFC 2782)
 */
struct SrvRecord {
    uint16_t priority;
    uint16_t weight;
    uint16_t port;
    std::string target;

    SrvRecord() : priority(0), weight(0), port(0) {}
    SrvRecord(uint16_t p, uint16_t w, uint16_t pt, const std::string& t)
        : priority(p), weight(w), port(pt), target(t) {}
};

} // namespace orb

#endif // ORB_SRV_RECORD_H

