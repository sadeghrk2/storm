#pragma once

#include <boost/container/flat_set.hpp>
#include <boost/optional/optional.hpp>
#include <stack>
#include <unordered_set>
#include <limits>

#include "storm/models/sparse/StateLabeling.h"
#include "storm/models/sparse/StandardRewardModel.h"
#include "storm/models/sparse/Model.h"
#include "storm/storage/SparseMatrix.h"
#include "storm/storage/sparse/StateStorage.h"

#include "storm-dft/builder/DftExplorationHeuristic.h"
#include "storm-dft/generator/DftNextStateGenerator.h"
#include "storm-dft/storage/dft/DFT.h"
#include "storm-dft/storage/dft/SymmetricUnits.h"
#include "storm-dft/storage/BucketPriorityQueue.h"

namespace storm {
    namespace builder {

        /*!
         * Build a Markov chain from DFT.
         */
        template<typename ValueType, typename StateType = uint32_t>
        class ExplicitDFTModelBuilderApprox {

            using DFTStatePointer = std::shared_ptr<storm::storage::DFTState<ValueType>>;
            // TODO Matthias: make choosable
            using ExplorationHeuristic = DFTExplorationHeuristicNone<ValueType>;
            using ExplorationHeuristicPointer = std::shared_ptr<ExplorationHeuristic>;


            // A structure holding the individual components of a model.
            struct ModelComponents {
                // Constructor
                ModelComponents();

                // The transition matrix.
                storm::storage::SparseMatrix<ValueType> transitionMatrix;

                // The state labeling.
                storm::models::sparse::StateLabeling stateLabeling;

                // The Markovian states.
                storm::storage::BitVector markovianStates;

                // The exit rates.
                std::vector<ValueType> exitRates;

                // A vector that stores a labeling for each choice.
                boost::optional<std::vector<boost::container::flat_set<StateType>>> choiceLabeling;

                // A flag indicating if the model is deterministic.
                bool deterministicModel;
            };

            // A class holding the information for building the transition matrix.
            class MatrixBuilder {
            public:
                // Constructor
                MatrixBuilder(bool canHaveNondeterminism);

                /*!
                 * Set a mapping from a state id to the index in the matrix.
                 *
                 * @param id Id of the state.
                 */
                void setRemapping(StateType id) {
                    STORM_LOG_ASSERT(id < stateRemapping.size(), "Invalid index for remapping.");
                    stateRemapping[id] = currentRowGroup;
                }

                /*!
                 * Create a new row group if the model is nondeterministic.
                 */
                void newRowGroup() {
                    if (canHaveNondeterminism) {
                        builder.newRowGroup(currentRow);
                    }
                    ++currentRowGroup;
                }

                /*!
                 * Add a transition from the current row.
                 *
                 * @param index Target index
                 * @param value Value of transition
                 */
                void addTransition(StateType index, ValueType value) {
                    builder.addNextValue(currentRow, index, value);
                }

                /*!
                 * Finish the current row.
                 */
                void finishRow() {
                    ++currentRow;
                }

                /*!
                 * Remap the columns in the matrix.
                 */
                void remap() {
                    builder.replaceColumns(stateRemapping, mappingOffset);
                }

                /*!
                 * Get the current row group.
                 *
                 * @return The current row group.
                 */
                StateType getCurrentRowGroup() {
                    return currentRowGroup;
                }

                /*!
                 * Get the remapped state for the given id.
                 *
                 * @param id State.
                 *
                 * @return Remapped index.
                 */
                StateType getRemapping(StateType id) {
                    STORM_LOG_ASSERT(id < stateRemapping.size(), "Invalid index for remapping.");
                    return stateRemapping[id];
                }

                // Matrix builder.
                storm::storage::SparseMatrixBuilder<ValueType> builder;

                // Offset to distinguish states which will not be remapped anymore and those which will.
                size_t mappingOffset;

                // A mapping from state ids to the row group indices in which they actually reside.
                // TODO Matthias: avoid hack with fixed int type
                std::vector<uint_fast64_t> stateRemapping;

            private:

                // Index of the current row group.
                StateType currentRowGroup;

                // Index of the current row.
                StateType currentRow;

                // Flag indicating if row groups are needed.
                bool canHaveNondeterminism;
            };

        public:
            // A structure holding the labeling options.
            struct LabelOptions {
                // Constructor
                LabelOptions(std::vector<std::shared_ptr<storm::logic::Formula const>> properties, bool buildAllLabels = false);

                // Flag indicating if the general fail label should be included.
                bool buildFailLabel;

                // Flag indicating if the general failsafe label should be included.
                bool buildFailSafeLabel;

                // Flag indicating if all possible labels should be included.
                bool buildAllLabels;

                // Set of element names whose fail label to include.
                std::set<std::string> elementLabels;
            };

            /*!
             * Constructor.
             *
             * @param dft DFT.
             * @param symmetries Symmetries in the dft.
             * @param enableDC Flag indicating if dont care propagation should be used.
             */
            ExplicitDFTModelBuilderApprox(storm::storage::DFT<ValueType> const& dft, storm::storage::DFTIndependentSymmetries const& symmetries, bool enableDC);

            /*!
             * Build model from DFT.
             *
             * @param labelOpts              Options for labeling.
             * @param iteration              Current number of iteration.
             * @param approximationThreshold Threshold determining when to skip exploring states.
             */
            void buildModel(LabelOptions const& labelOpts, size_t iteration, double approximationThreshold = 0.0);

            /*!
             * Get the built model.
             *
             * @return The model built from the DFT.
             */
            std::shared_ptr<storm::models::sparse::Model<ValueType>> getModel();

            /*!
             * Get the built approximation model for either the lower or upper bound.
             *
             * @param lowerBound If true, the lower bound model is returned, else the upper bound model
             *
             * @return The model built from the DFT.
             */
            std::shared_ptr<storm::models::sparse::Model<ValueType>> getModelApproximation(bool lowerBound = true);

        private:

            /*!
             * Explore state space of DFT.
             *
             * @param approximationThreshold Threshold to determine when to skip states.
             */
            void exploreStateSpace(double approximationThreshold);

            /*!
             * Initialize the matrix for a refinement iteration.
             */
            void initializeNextIteration();

            /*!
             * Build the labeling.
             *
             * @param labelOpts Options for labeling.
             */
            void buildLabeling(LabelOptions const& labelOpts);

            /*!
             * Add a state to the explored states (if not already there). It also handles pseudo states.
             *
             * @param state The state to add.
             *
             * @return Id of state.
             */
            StateType getOrAddStateIndex(DFTStatePointer const& state);

            /*!
             * Set markovian flag for the current state.
             *
             * @param markovian Flag indicating if the state is markovian.
             */
            void setMarkovian(bool markovian);

            /**
             * Change matrix to reflect the lower or upper approximation bound.
             *
             * @param matrix     Matrix to change. The change are reflected here.
             * @param lowerBound Flag indicating if the lower bound should be used. Otherwise the upper bound is used.
             */
            void changeMatrixBound(storm::storage::SparseMatrix<ValueType> & matrix, bool lowerBound) const;

            /*!
             * Get lower bound approximation for state.
             *
             * @return Lower bound approximation.
             */
            ValueType getLowerBound(DFTStatePointer const& state) const;

            /*!
             * Get upper bound approximation for state.
             *
             * @return Upper bound approximation.
             */
            ValueType getUpperBound(DFTStatePointer const& state) const;

            /*!
             * Compute the MTTF of an AND gate via a closed formula.
             * The used formula is 1/( 1/a + 1/b + 1/c + ... - 1/(a+b) - 1/(a+c) - ... + 1/(a+b+c) + ... - ...)
             *
             * @param rates List of rates of children of AND.
             * @param size  Only indices < size are considered in the vector.
             * @return MTTF.
             */
            ValueType computeMTTFAnd(std::vector<ValueType> const& rates, size_t size) const;

            /*!
             * Compares the priority of two states.
             *
             * @param idA Id of first state
             * @param idB Id of second state
             *
             * @return True if the priority of the first state is greater then the priority of the second one.
             */
            bool isPriorityGreater(StateType idA, StateType idB) const;

            void printNotExplored() const;

            /*!
             * Create the model model from the model components.
             *
             * @param copy If true, all elements of the model component are copied (used for approximation). If false
             *             they are moved to reduce the memory overhead.
             *
             * @return The model built from the model components.
             */
            std::shared_ptr<storm::models::sparse::Model<ValueType>> createModel(bool copy);

            // Initial size of the bitvector.
            const size_t INITIAL_BITVECTOR_SIZE = 20000;
            // Offset used for pseudo states.
            const StateType OFFSET_PSEUDO_STATE = std::numeric_limits<StateType>::max() / 2;

            // Dft
            storm::storage::DFT<ValueType> const& dft;

            // General information for state generation
            // TODO Matthias: use const reference
            std::shared_ptr<storm::storage::DFTStateGenerationInfo> stateGenerationInfo;

            // Flag indication if dont care propagation should be used.
            bool enableDC = true;

            //TODO Matthias: make changeable
            const bool mergeFailedStates = false;

            // Heuristic used for approximation
            storm::builder::ApproximationHeuristic usedHeuristic;

            // Current id for new state
            size_t newIndex = 0;

            // Id of failed state
            size_t failedStateId = 0;

            // Id of initial state
            size_t initialStateIndex = 0;

            // Next state generator for exploring the state space
            storm::generator::DftNextStateGenerator<ValueType, StateType> generator;

            // Structure for the components of the model.
            ModelComponents modelComponents;

            // Structure for the transition matrix builder.
            MatrixBuilder matrixBuilder;

            // Internal information about the states that were explored.
            storm::storage::sparse::StateStorage<StateType> stateStorage;

            // A priority queue of states that still need to be explored.
            storm::storage::BucketPriorityQueue<ValueType> explorationQueue;

            // A mapping of not yet explored states from the id to the tuple (state object, heuristic values).
            std::map<StateType, std::pair<DFTStatePointer, ExplorationHeuristicPointer>> statesNotExplored;

            // Holds all skipped states which were not yet expanded. More concretely it is a mapping from matrix indices
            // to the corresponding skipped states.
            // Notice that we need an ordered map here to easily iterate in increasing order over state ids.
            // TODO remove again
            std::map<StateType, std::pair<DFTStatePointer, ExplorationHeuristicPointer>> skippedStates;

            // List of independent subtrees and the BEs contained in them.
            std::vector<std::vector<size_t>> subtreeBEs;
        };

    }
}
