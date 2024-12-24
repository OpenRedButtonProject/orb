/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
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

#include "DataBuffer.h"

using aidl::org::orbtv::orbservice::DataBuffer;

//binder_status_t AParcel_writeByteArray(AParcel* parcel, const int8_t* arrayData, int32_t length)
//binder_status_t AParcel_readByteArray(const AParcel* parcel, void* arrayData, AParcel_byteArrayAllocator allocator)

DataBuffer::DataBuffer(int32_t size, int8_t* data)
{
   m_data = new int8_t[size];
   if (m_data != nullptr)
   {
      std::copy(data, data + size, m_data);
      m_size = size;
   }
   else
   {
      m_size = 0;
   }
}

DataBuffer::~DataBuffer()
{
   if (m_data != nullptr)
   {
      delete[] m_data;
   }
}

static bool byteArrayAllocator(void *arrayData, int32_t length, int8_t **outBuffer)
{
   auto *db = static_cast<DataBuffer *>(arrayData);
   bool result;
   auto *buffer = new int8_t[length];
   if (buffer != nullptr)
   {
      result = true;
      db->setSize(length);
   }
   else
   {
      result = false;
   }
   *outBuffer = buffer;
   return result;
}

binder_status_t DataBuffer::readFromParcel(const AParcel* pParcel)
{
   binder_status_t status;
   status = AParcel_readByteArray(pParcel, this, byteArrayAllocator);
   return status;
}

binder_status_t DataBuffer::writeToParcel(AParcel* pParcel) const
{
   binder_status_t status;
   status = AParcel_writeByteArray(pParcel, m_data, m_size);
   return status;
}

std::string DataBuffer::toString() const {
    std::string str("DataBuffer");
    return str;
}
