#ifndef OPENSSL_HASH_CALCULATOR_H
#define OPENSSL_HASH_CALCULATOR_H

#include "OpAppPackageManager.h"

class OpenSSLHashCalculator : public IHashCalculator {
public:
  std::string calculateSHA256Hash(const std::string& filePath) const override;
};

#endif /* OPENSSL_HASH_CALCULATOR_H */
