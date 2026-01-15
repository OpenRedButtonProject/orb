#ifndef HASH_CALCULATOR_H
#define HASH_CALCULATOR_H

#include "IHashCalculator.h"

namespace orb
{

class HashCalculator : public IHashCalculator {
public:
    std::string calculateSHA256Hash(const std::filesystem::path& filePath) const override;
};

} // namespace orb

#endif /* HASH_CALCULATOR_H */
