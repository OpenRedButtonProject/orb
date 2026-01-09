#include "Decryptor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef IS_CHROMIUM
#include "third_party/boringssl/src/include/openssl/sha.h"
#else
#include <openssl/sha.h>
#endif

namespace orb
{

bool Decryptor::decrypt(
  const std::filesystem::path& filePath,
  std::filesystem::path& outFile,
  std::string& outError) const
{
  // Default implementation: pass-through (no actual decryption)
  outFile = filePath;
  outError.clear();
  return true;
}

} // namespace orb
