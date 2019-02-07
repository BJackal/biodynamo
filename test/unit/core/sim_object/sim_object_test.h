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

#ifndef UNIT_CORE_SIM_OBJECT_TEST_H_
#define UNIT_CORE_SIM_OBJECT_TEST_H_

#include <gtest/gtest.h>
#include "core/sim_object/cell.h"
#include "core/biology_module/biology_module.h"
#include "core/sim_object/sim_object.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {
namespace sim_object_test_internal {

struct GrowthModule : public BaseBiologyModule {
  double growth_rate_ = 0.5;
  GrowthModule() : BaseBiologyModule(CellDivisionEvent::kEventId) {}

  virtual ~GrowthModule() {}

  BaseBiologyModule* GetInstance() const { return new GrowthModule(); }
  BaseBiologyModule* GetCopy() const { return new GrowthModule(*this); }

  void EventConstructor(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) override {
    BaseBiologyModule::EventConstructor(event, other, new_oid);
    if(GrowthModule* gbm = dynamic_cast<GrowthModule*>(other)) {
      growth_rate_ = gbm->growth_rate_;
    } else {
      Log::Fatal("GrowthModule::EventConstructor", "other was not of type GrowthModule");
    }
  }

  /// Default event handler (exising biology module won't be modified on
  /// any event)
  void EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2 = nullptr) override {
    BaseBiologyModule::EventHandler(event, other1, other2);
  }

  void Run(SimObject* t) override {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  BDM_CLASS_DEF(GrowthModule, 1);
};

struct MovementModule : public BaseBiologyModule {
  std::array<double, 3> velocity_;

  MovementModule() : BaseBiologyModule(0, CellDivisionEvent::kEventId), velocity_({{0, 0, 0}}) {}
  explicit MovementModule(const std::array<double, 3>& velocity)
      : BaseBiologyModule(0, CellDivisionEvent::kEventId), velocity_(velocity) {}

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance() const { return new MovementModule(); }
  BaseBiologyModule* GetCopy() const { return new MovementModule(*this); }

  void EventConstructor(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) override {
    BaseBiologyModule::EventConstructor(event, other, new_oid);
    if(MovementModule* mbm = dynamic_cast<MovementModule*>(other)) {
      velocity_ = mbm->velocity_;
    } else {
      Log::Fatal("MovementModule::EventConstructor", "other was not of type MovementModule");
    }
  }

  /// Default event handler
  void EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2 = nullptr) override {
    BaseBiologyModule::EventHandler(event, other1, other2);
  }

  void Run(SimObject* so) override {
    const auto& position = so->GetPosition();
    so->SetPosition(Math::Add(position, velocity_));
  }

  BDM_CLASS_DEF(MovementModule, 1);
};

/// This biology module removes itself the first time it is executed
struct RemoveModule : public BaseBiologyModule {
  RemoveModule() {}

  BaseBiologyModule* GetInstance() const { return new RemoveModule(); }
  BaseBiologyModule* GetCopy() const { return new RemoveModule(*this); }

  void Run(SimObject* sim_object) override {
    sim_object->RemoveBiologyModule(this);
  }

  BDM_CLASS_DEF(RemoveModule, 1);
};

}  // namespace sim_object_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_SIM_OBJECT_TEST_H_
