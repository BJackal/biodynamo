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

#ifndef UNIT_TEST_UTIL_TEST_SIM_OBJECT_H_
#define UNIT_TEST_UTIL_TEST_SIM_OBJECT_H_

#include <set>
#include <string>
#include "core/sim_object/sim_object.h"

namespace bdm {

class TestSimObject : public SimObject {
  BDM_SIM_OBJECT_HEADER(TestSimObject, SimObject, 1, position_, diameter_);

 public:
  static std::set<std::string> GetRequiredVisDataMembers() {
    return {"diameter_", "position_"};
  }

  TestSimObject() {}

  TestSimObject(int data) : data_(data) {}

  explicit TestSimObject(const std::array<double, 3>& pos)
      : position_{pos} {}

  virtual ~TestSimObject() {}

  Shape GetShape() const override { return Shape::kSphere; };

  void RunDiscretization() override {}

  const std::array<double, 3> GetPosition() const override { return position_; }

  void SetPosition(const std::array<double, 3>& pos) override { position_ = pos; }

  void ApplyDisplacement(const std::array<double, 3>&) override {}

  std::array<double, 3> CalculateDisplacement(double squared_radius) override {
    return {0, 0, 0};
  }

  double GetDiameter() const override { return diameter_; }
  void SetDiameter(const double diameter) override { diameter_ = diameter; }

  int GetData() const { return data_; }
  void SetData(double data) { data_ = data; }

 protected:
  std::array<double, 3> position_ = {{0, 0, 0}};
  double diameter_ = 0;
  int data_ = 0;
};

}  // namespace bdm

#endif  // UNIT_TEST_UTIL_TEST_SIM_OBJECT_H_
