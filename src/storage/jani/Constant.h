#pragma once

#include <string>

#include <boost/optional.hpp>

#include "src/storage/expressions/Variable.h"
#include "src/storage/expressions/Expression.h"

namespace storm {
    namespace jani {
        
        class Constant {
        public:
            /*!
             * Creates a constant.
             */
            Constant(std::string const& name, boost::optional<storm::expressions::Expression> const& expression = boost::none);
            
            /*!
             * Retrieves whether the constant is defined in the sense that it has a defining expression.
             */
            bool isDefined() const;
            
            /*!
             * Defines the constant with the given expression.
             */
            void define(storm::expressions::Expression const& expression);
            
            /*!
             * Retrieves the type of the constant.
             */
            storm::expressions::Type const& getType() const;
            
            /*!
             * Retrieves whether the constant is a boolean constant.
             */
            bool isBooleanConstant() const;
            
            /*!
             * Retrieves whether the constant is an integer constant.
             */
            bool isIntegerConstant() const;
            
            /*!
             * Retrieves whether the constant is a real constant.
             */
            bool isRealConstant() const;
            
        private:
            // The name of the constant.
            std::string name;
            
            // The expression variable associated with the constant.
            storm::expressions::Variable variable;

            // The expression defining the constant (if any).
            boost::optional<storm::expressions::Expression> expression;
        };
        
    }
}