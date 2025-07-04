#ifndef OPENSSL_DECRYPTOR_H
#define OPENSSL_DECRYPTOR_H

#include "OpAppPackageManager.h"

class OpenSSLDecryptor : public IDecryptor {
public:
  PackageOperationResult decrypt(const std::string& filePath) const override;
};

#endif /* OPENSSL_DECRYPTOR_H */
