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

#include <binder/Parcel.h>

#include "DataBuffer.h"

using org::orbtv::orbservice::DataBuffer;

DataBuffer::DataBuffer(uint32_t size, uint8_t* data)
{
   // TODO
}

DataBuffer::~DataBuffer()
{
}

status_t DataBuffer::readFromParcel(const Parcel* pParcel)
{
   status_t status;
   status = pParcel->readByteVector(&m_data);
   return status;
}

status_t DataBuffer::writeToParcel(Parcel* pParcel) const
{
   status_t status;
   status = pParcel->writeByteVector(m_data);
   return status;
}

std::string DataBuffer::toString() const {
    std::string str("DataBuffer");
    return str;
}
