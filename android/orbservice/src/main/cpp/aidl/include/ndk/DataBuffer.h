#ifndef DATABUFFER_H_
#define DATABUFFER_H_

#include <android/binder_auto_utils.h>
#include <android/binder_parcel.h>
#include <android/binder_status.h>

namespace aidl::org::orbtv::orbservice {

class DataBuffer
{
public:
   DataBuffer() : m_data(nullptr), m_size(0) {}

   DataBuffer(int32_t size, int8_t* data);
   ~DataBuffer();

public:
    binder_status_t readFromParcel(const AParcel* pParcel);
    binder_status_t writeToParcel(AParcel* pParcel) const;
    [[nodiscard]] std::string toString() const;
    void setSize(int32_t size) { m_size = size; }

private:
   int8_t* m_data;
   int32_t m_size;
};

} // namespace aidl::org::orbtv::orbservice

#endif //DATABUFFER_H_
