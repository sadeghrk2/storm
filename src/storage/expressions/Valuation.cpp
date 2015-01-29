#include "src/storage/expressions/Valuation.h"
#include "src/storage/expressions/ExpressionManager.h"

namespace storm {
    namespace expressions {
        Valuation::Valuation(std::shared_ptr<ExpressionManager const> const& manager) : manager(manager) {
            // Intentionally left empty.
        }
        
        Valuation::~Valuation() {
            // Intentionally left empty.
        }
        
        ExpressionManager const& Valuation::getManager() const {
            return *manager;
        }
        
        void Valuation::setManager(std::shared_ptr<ExpressionManager const> const& manager) {
            this->manager = manager;
        }

    }
}