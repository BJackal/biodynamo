#include "synapse/test_synapses.h"

#include "local_biology/neurite_element.h"

namespace cx3d {
namespace synapse {

void TestSynapses::extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<physics::ECM>& ecm) {
  extendExcressencesAndSynapseOnEveryNeuriteElement(ecm, 1.0);
}

void TestSynapses::extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<physics::ECM>& ecm,
                                                                     double probability_to_synapse) {
  for (auto ne : ecm->getNeuriteElementList()) {
    if (ne->isAxon()) {
      ne->makeBoutons(2);
    } else {
      ne->makeSpines(5);
    }
  }
  for (auto ne : ecm->getNeuriteElementList()) {
    if (ne->isAxon()) {
      ne->synapseBetweenExistingBS(probability_to_synapse);
    }
  }
}

}  // namespace synapse
}  // namespace cx3d
