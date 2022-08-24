/**
 * The MIT License (MIT)
 * Copyright (c) 2016 tomykaira
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include <string>

namespace orb {

/**
 * @brief orb::Base64
 *
 * Implementation of base64 encoding/decoding.
 */
class Base64 {

public:

  /**
   * Base64::Encode
   *
   * Encode the specified data using the base64 encoding algorithm.
   *
   * @param data The specified data
   * 
   * @return The encoded data
   */
  static std::string Encode(const std::string data);
  
  /**
   * Base64::Decode
   *
   * Decode the specified data using the base64 decoding algorithm.
   *
   * @param input (in)  The base64-encoded data
   * @param out   (out) The decoded data
   * 
   * @return An empty string in success, or else an error message
   */
  static std::string Decode(const std::string& input, std::string& out);

}; // class Base64

} // namespace orb
