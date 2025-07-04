#include "OpenSSLHashCalculator.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef IS_CHROMIUM
#include "third_party/boringssl/src/include/openssl/sha.h"
#else
#include <openssl/sha.h> // TODO: use local ssl library instead
#endif

std::string OpenSSLHashCalculator::calculateSHA256Hash(const std::string& filePath) const
{
  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    return ""; // Return empty string if file cannot be opened
  }

  // Initialize SHA256 context
  SHA256_CTX sha256;
  SHA256_Init(&sha256);

  // Read file in chunks and update hash
  const size_t bufferSize = 4096; // 4KB chunks
  std::vector<char> buffer(bufferSize);

  while (file.read(buffer.data(), bufferSize)) {
    SHA256_Update(&sha256, buffer.data(), file.gcount());
  }

  // Handle any remaining bytes
  if (file.gcount() > 0) {
    SHA256_Update(&sha256, buffer.data(), file.gcount());
  }

  // Finalize the hash
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_Final(hash, &sha256);

  // Convert hash to hexadecimal string
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::setw(2) << static_cast<int>(hash[i]);
  }

  return ss.str();
}
