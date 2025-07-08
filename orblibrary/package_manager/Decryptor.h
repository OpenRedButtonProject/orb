#ifndef DECRYPTOR_H
#define DECRYPTOR_H

#include "OpAppPackageManager.h"

class Decryptor : public IDecryptor {
public:
  PackageOperationResult decrypt(const std::string& filePath) const override;
};

#endif /* DECRYPTOR_H */
