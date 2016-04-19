#ifndef SYNAPSE_TEST_SYNAPSES_H_
#define SYNAPSE_TEST_SYNAPSES_H_

#include <memory>

#include "physics/ecm.h"

namespace cx3d {
namespace synapse {

class TestSynapses {
 public:
  static void extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<physics::ECM>& ecm);

  static void extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<physics::ECM>& ecm,
                                                                double probability_to_synapse);
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_TEST_SYNAPSES_H_
