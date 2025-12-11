#ifndef DECRYPTOR_H
#define DECRYPTOR_H

#include "OpAppPackageManager.h"

namespace orb
{

class Decryptor : public IDecryptor {
public:
  PackageOperationResult decrypt(const std::string& filePath) const override;
};

} // namespace orb

#endif /* DECRYPTOR_H */
