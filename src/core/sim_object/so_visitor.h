// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_SIM_OBJECT_SO_VISITOR_H_
#define CORE_SIM_OBJECT_SO_VISITOR_H_

#include <atomic>

namespace bdm {

// TODO documentation
struct SoVisitor {
  virtual ~SoVisitor() {}
  virtual void Visit(const std::string& name, size_t type_hash_code, void* data) = 0;
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_VISITOR_H_
