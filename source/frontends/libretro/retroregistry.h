#pragma once

#include <memory>

class Registry;

namespace ra2
{

  void SetupRetroVariables();
  std::shared_ptr<Registry> CreateRetroRegistry();
  u_int32_t lookupValue(std::string section, std::string key);
}
