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

#ifndef CORE_BIOLOGY_MODULE_BIOLOGY_MODULE_H_
#define CORE_BIOLOGY_MODULE_BIOLOGY_MODULE_H_

#include "core/event/event.h"
#include "core/util/root.h"

namespace bdm {

class SimObject;

/// BaseBiologyModule encapsulates logic to decide for which EventIds
/// a biology module should be copied or removed
struct BaseBiologyModule {
  /// Default ctor sets `copy_mask_` and remove_mask_` to 0; meaning that
  /// `Copy` and `Remove` will always return false
  BaseBiologyModule();

  explicit BaseBiologyModule(EventId copy_event, EventId remove_event = 0);

  BaseBiologyModule(std::initializer_list<EventId> copy_events,
                    std::initializer_list<EventId> remove_events = {});

  BaseBiologyModule(const Event& event, BaseBiologyModule* other,
                    uint64_t new_oid = 0);

  BaseBiologyModule(const BaseBiologyModule& other);

  virtual ~BaseBiologyModule();

  /// Create a new instance of this object using the default constructor.
  virtual BaseBiologyModule* GetInstance(const Event& event,
                                         BaseBiologyModule* other,
                                         uint64_t new_oid = 0) const = 0;

  /// Create a copy of this biology module.
  virtual BaseBiologyModule* GetCopy() const = 0;

  virtual void EventHandler(const Event& event, BaseBiologyModule* other1,
                            BaseBiologyModule* other2 = nullptr);

  virtual void Run(SimObject* so) = 0;

  /// Function returns whether the biology module should be copied for the
  /// given event.
  bool Copy(EventId event) const;

  /// Function returns whether the biology module should be removed for the
  /// given event.
  bool Remove(EventId event) const;

 private:
  EventId copy_mask_;
  EventId remove_mask_;
  BDM_CLASS_DEF(BaseBiologyModule, 2);
};

/// Inserts boilerplate code for stateless biology modules
#define BDM_STATELESS_BM_HEADER(class_name, base_class, class_version_id)      \
 public:                                                                       \
  /** Empty default event constructor, because Chemotaxis does not have state. \
   */                                                                          \
  class_name(const Event& event, BaseBiologyModule* other,                     \
             uint64_t new_oid = 0)                                             \
      : base_class(event, other, new_oid) {}                                   \
                                                                               \
  /** Event handler not needed, because Chemotaxis does not have state. */     \
                                                                               \
  /** Create a new instance of this object using the default constructor. */   \
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other, \
                                 uint64_t new_oid = 0) const override {        \
    return new class_name(event, other, new_oid);                              \
  }                                                                            \
                                                                               \
  /** Create a copy of this biology module. */                                 \
  BaseBiologyModule* GetCopy() const override {                                \
    return new class_name(*this);                                              \
  }                                                                            \
                                                                               \
 private:                                                                      \
  BDM_CLASS_DEF(class_name, class_version_id);

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_BIOLOGY_MODULE_H_
