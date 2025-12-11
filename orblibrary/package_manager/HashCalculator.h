#ifndef HASH_CALCULATOR_H
#define HASH_CALCULATOR_H

#include "OpAppPackageManager.h"

namespace orb
{

class HashCalculator : public IHashCalculator {
public:
  std::string calculateSHA256Hash(const std::string& filePath) const override;
};

} // namespace orb

#endif /* HASH_CALCULATOR_H */
