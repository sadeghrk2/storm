#pragma once

#include "storm-dft/storage/dft/DFT.h"
#include "storm-gspn/storage/gspn/GSPN.h"
#include "storm-gspn/storage/gspn/GspnBuilder.h"

namespace storm {
    namespace transformations {
        namespace dft {

            /*!
             * Transformator for DFT -> GSPN.
             */
            template<typename ValueType>
            class DftToGspnTransformator {

            public:
                /*!
                 * Constructor.
                 *
                 * @param dft DFT
                 */
                DftToGspnTransformator(storm::storage::DFT<ValueType> const& dft);

                /*!
                 * Transform the DFT to a GSPN.
                 */
                void transform(bool smart = true);

                /*!
                 * Extract Gspn by building
                 *
                 */
                gspn::GSPN* obtainGSPN();
                
                /*!
                 * Get failed place id of top level element.
                 */
                uint64_t toplevelFailedPlaceId();
                
            private:
                /*!
				 * Translate all elements of the GSPN.
				 */
				void translateGSPNElements();

				/*!
				 * Translate a GSPN Basic Event.
				 * 
				 * @param dftBE The Basic Event.
				 */
				void translateBE(std::shared_ptr<storm::storage::DFTBE<ValueType> const> dftBE);

                /*!
                 * Translate a GSPN CONSTF (Constant Failure, a Basic Event that has already failed).
                 *
                 * @param dftPor The CONSTF Basic Event.
                 */
                void translateCONSTF(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dftConstF);

                /*!
                 * Translate a GSPN CONSTS (Constant Save, a Basic Event that cannot fail).
                 *
                 * @param dftPor The CONSTS Basic Event.
                 */
                void translateCONSTS(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dftConstS);

				/*!
				 * Translate a GSPN AND.
				 * 
				 * @param dftAnd The AND gate.
				 */
				void translateAND(std::shared_ptr<storm::storage::DFTAnd<ValueType> const> dftAnd);
				
				/*!
				 * Translate a GSPN OR.
				 * 
				 * @param dftOr The OR gate.
				 */
				void translateOR(std::shared_ptr<storm::storage::DFTOr<ValueType> const> dftOr);
				
				/*!
				 * Translate a GSPN VOT.
				 * 
				 * @param dftVot The VOT gate.
				 */
				void translateVOT(std::shared_ptr<storm::storage::DFTVot<ValueType> const> dftVot);
				
				/*!
				 * Translate a GSPN PAND.
				 *
				 * @param dftPand The PAND gate.
                 * @param inclusive Flag wether the PAND is inclusive (children are allowed to fail simultaneously and the PAND will fail nevertheless)
				 */
				void translatePAND(std::shared_ptr<storm::storage::DFTPand<ValueType> const> dftPand, bool inclusive = true);

                /*!
                 * Translate a GSPN POR.
                 *
                 * @param dftPor The POR gate.
                 * @param inclusive Flag wether the POR is inclusive (children are allowed to fail simultaneously and the POR will fail nevertheless)
                 */
                void translatePOR(std::shared_ptr<storm::storage::DFTPor<ValueType> const> dftPor, bool inclusive = true);

				/*!
				 * Translate a GSPN SPARE.
				 * 
				 * @param dftSpare The SPARE gate.
				 */
				void translateSPARE(std::shared_ptr<storm::storage::DFTSpare<ValueType> const> dftSpare);

				/*!
				 * Translate a GSPN PDEP (FDEP is included with a probability of 1).
                 *
                 * @param dftDependency The PDEP gate.
                 */
				void translatePDEP(std::shared_ptr<storm::storage::DFTDependency<ValueType> const> dftDependency);
				
                /*!
                 * Translate a GSPN SEQ.
                 *
                 * @param dftSeq The SEQ gate.
                 */
                void translateSeq(std::shared_ptr<storm::storage::DFTSeq<ValueType> const> dftSeq);

				/*!
				 * Check if the element is active intially.
				 *
				 * @param dFTElement DFT element.
                 *
                 * @return True iff element is active initially.
				 */
                bool isActiveInitially(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dFTElement);
				 
				/*!
				 * Get the priority of the element.
				 * The priority is two times the length of the shortest path to the top event.
				 *
				 * @param priority The priority of the gate. Top Event has priority 0,  its children 2, its grandchildren 4, ...
				 *
				 * @param dftElement The element whose priority shall be determined.
				 */
                uint64_t getFailPriority(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dFTElement);

                /*!
                 * Add failed place for an element.
                 *
                 * @param dftElement Element.
                 * @param layoutInfo Information about layout.
                 * @param initialFailed Flag indicating whether the element is initially failed.
                 *
                 * @return Id of added failed place.
                 */
                uint64_t addFailedPlace(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dftElement, storm::gspn::LayoutInfo const& layoutInfo, bool initialFailed = false);

                /*!
                 * Add unavailable place for element.
                 *
                 * @param dftElement Element.
                 * @param layoutInfo Information about layout.
                 * @param initialAvailable Flag indicating whether the element is available initially.
                 *
                 * @return Id of added unavailable place.
                 */
                uint64_t addUnavailablePlace(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dftElement, storm::gspn::LayoutInfo const& layoutInfo, bool initialAvailable = true);

                /*!
                 * Add disabled place for element.
                 *
                 * @param dftBe Basic Element.
                 * @param layoutInfo Information about layout.
                 *
                 * @return Id of added disabled place.
                 */
                uint64_t addDisabledPlace(std::shared_ptr<storm::storage::DFTBE<ValueType> const> dftBe, storm::gspn::LayoutInfo const& layoutInfo);

                /*!
                 * Get failed place for element.
                 *
                 * @param dftElement Element.
                 *
                 * @return Id of failed place corresponding to the given element.
                 */
                uint64_t getFailedPlace(std::shared_ptr<storm::storage::DFTElement<ValueType> const> dftElement) {
                    return failedPlaces.at(dftElement->id());
                }
                
                storm::storage::DFT<ValueType> const& mDft;
                storm::gspn::GspnBuilder builder;

                // Interface places for DFT elements
                std::vector<uint64_t> failedPlaces;
                std::map<uint64_t, uint64_t> unavailablePlaces;
                std::map<uint64_t, uint64_t> activePlaces;
                std::map<uint64_t, uint64_t> disabledPlaces;
                bool smart;
                
                static constexpr const char* STR_FAILING = "_failing";			// Name standard for transitions that point towards a place, which in turn indicates the failure of a gate.
                static constexpr const char* STR_FAILED = "_failed";			// Name standard for place which indicates the failure of a gate.
                static constexpr const char* STR_FAILSAVING = "_failsaving";	// Name standard for transition that point towards a place, which in turn indicates the failsave state of a gate.
                static constexpr const char* STR_FAILSAVE = "_failsave";		// Name standard for place which indicates the failsave state of a gate.
                static constexpr const char* STR_ACTIVATING = "_activating";	// Name standard for transition that point towards a place, which in turn indicates its activity.
                static constexpr const char* STR_ACTIVATED = "_active";         // Name standard for place which indicates the activity.
            };
        }
    }
}

