#ifndef DATABUFFER_H_
#define DATABUFFER_H_

#include <binder/Parcelable.h>

using android::status_t;
using android::Parcel;

namespace org::orbtv::orbservice {

class DataBuffer : public android::Parcelable
{
public:
   DataBuffer() : m_data(), m_size(0) {}
   DataBuffer(uint32_t size, uint8_t* data);
   ~DataBuffer();

public:
    status_t readFromParcel(const Parcel* pParcel) override;
    status_t writeToParcel(Parcel* pParcel) const override;
    std::string toString() const;

private:
   std::vector<uint8_t> m_data;
   uint32_t m_size;
};

} // namespace org::orbtv::orbservice

#endif //DATABUFFER_H_
