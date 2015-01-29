#ifndef STORM_LOGIC_ATOMICLABELFORMULA_H_
#define STORM_LOGIC_ATOMICLABELFORMULA_H_

#include <string>

#include "src/logic/StateFormula.h"

namespace storm {
    namespace logic {
        class AtomicLabelFormula : public StateFormula {
        public:
            AtomicLabelFormula(std::string const& label);
            
            virtual ~AtomicLabelFormula() {
                // Intentionally left empty.
            }
            
            virtual bool isAtomicLabelFormula() const override;

            virtual bool isPctlStateFormula() const override;
            virtual bool isLtlFormula() const override;
            virtual bool isPropositionalFormula() const override;
            
            std::string const& getLabel() const;
            
            virtual void gatherAtomicLabelFormulas(std::vector<std::shared_ptr<AtomicLabelFormula const>>& atomicLabelFormulas) const override;
            
            virtual std::ostream& writeToStream(std::ostream& out) const override;
            
        private:
            std::string label;
        };
    }
}

#endif /* STORM_LOGIC_ATOMICLABELFORMULA_H_ */