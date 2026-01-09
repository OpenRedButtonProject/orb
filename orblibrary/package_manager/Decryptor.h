#ifndef DECRYPTOR_H
#define DECRYPTOR_H

#include "OpAppPackageManager.h"

namespace orb
{

class Decryptor : public IDecryptor {
public:
  bool decrypt(
    const std::filesystem::path& filePath,
    std::filesystem::path& outFile,
    std::string& outError) const override;
};

} // namespace orb

#endif /* DECRYPTOR_H */
