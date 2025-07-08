#ifndef HASH_CALCULATOR_H
#define HASH_CALCULATOR_H

#include "OpAppPackageManager.h"

class HashCalculator : public IHashCalculator {
public:
  std::string calculateSHA256Hash(const std::string& filePath) const override;
};

#endif /* HASH_CALCULATOR_H */
