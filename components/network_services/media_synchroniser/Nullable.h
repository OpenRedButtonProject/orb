/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 *
 * NOTICE: This file has been created by Ocean Blue Software and is based on
 * the original work (https://github.com/bbc/pydvbcss) of the British
 * Broadcasting Corporation, as part of a translation of that work from a
 * Python library/tool to a native service. The following is the copyright
 * notice of the original work:
 *
 * Copyright 2015 British Broadcasting Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WIP_DVBCSS_HBBTV_NULLABLE_H
#define WIP_DVBCSS_HBBTV_NULLABLE_H

template <class T>
class Nullable {
public:
   Nullable() : m_isNull(true)
   {
   }

   Nullable(const T &value) : m_value(value), m_isNull(false)
   {
   }

   Nullable(const Nullable &other)
   {
      *this = other;
   }

   bool isNull() const
   {
      return m_isNull;
   }

   T value() const
   {
      return m_value;
   }

   Nullable &operator=(const T &value)
   {
      m_value = value;
      m_isNull = false;
      return *this;
   }

   Nullable &operator=(const Nullable &other)
   {
      m_value = other.m_value;
      m_isNull = other.m_isNull;
      return *this;
   }

   bool operator==(const Nullable &other) const
   {
      return this->m_isNull == other.m_isNull && this->m_value == other.m_value;
   }

   bool operator!=(const Nullable &other) const
   {
      return !(*this == other);
   }

private:
   T m_value;
   bool m_isNull;
};

#endif //WIP_DVBCSS_HBBTV_NULLABLE_H

