#include "storm-dft/api/storm-dft.h"

namespace storm {
    namespace api {

        template<>
        void exportDFTToJson(storm::storage::DFT<double> const& dft, std::string const& file) {
            storm::storage::DftJsonExporter<double>::toFile(dft, file);
        }

        template<>
        void exportDFTToJson(storm::storage::DFT<storm::RationalFunction> const& dft, std::string const& file) {
            STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Export to JSON not supported for this data type.");
        }

        template<>
        void exportDFTToSMT(storm::storage::DFT<double> const& dft, std::string const& file) {
            storm::modelchecker::DFTASFChecker asfChecker(dft);
            asfChecker.convert();
            asfChecker.toFile(file);
        }

        template<>
        void exportDFTToSMT(storm::storage::DFT<storm::RationalFunction> const& dft, std::string const& file) {
            STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Export to SMT does not support this data type.");
        }

        template<>
        void transformToGSPN(storm::storage::DFT<double> const& dft) {
            // Transform to GSPN
            storm::transformations::dft::DftToGspnTransformator<double> gspnTransformator(dft);
            bool smart = true;
            gspnTransformator.transform(smart);
            storm::gspn::GSPN* gspn = gspnTransformator.obtainGSPN();
            uint64_t toplevelFailedPlace = gspnTransformator.toplevelFailedPlaceId();

            storm::api::handleGSPNExportSettings(*gspn);

            std::shared_ptr<storm::expressions::ExpressionManager> const& exprManager = gspn->getExpressionManager();
            storm::builder::JaniGSPNBuilder builder(*gspn);
            storm::jani::Model* model =  builder.build();
            storm::jani::Variable const& topfailedVar = builder.getPlaceVariable(toplevelFailedPlace);

            storm::expressions::Expression targetExpression = exprManager->integer(1) == topfailedVar.getExpressionVariable().getExpression();
            auto evtlFormula = std::make_shared<storm::logic::AtomicExpressionFormula>(targetExpression);
            auto tbFormula = std::make_shared<storm::logic::BoundedUntilFormula>(std::make_shared<storm::logic::BooleanLiteralFormula>(true), evtlFormula, storm::logic::TimeBound(false, exprManager->integer(0)), storm::logic::TimeBound(false, exprManager->integer(10)), storm::logic::TimeBoundReference(storm::logic::TimeBoundType::Time));
            auto tbUntil = std::make_shared<storm::logic::ProbabilityOperatorFormula>(tbFormula);

            auto evFormula = std::make_shared<storm::logic::EventuallyFormula>(evtlFormula, storm::logic::FormulaContext::Time);
            auto rewFormula = std::make_shared<storm::logic::TimeOperatorFormula>(evFormula, storm::logic::OperatorInformation(), storm::logic::RewardMeasureType::Expectation);

            storm::settings::modules::JaniExportSettings const& janiSettings = storm::settings::getModule<storm::settings::modules::JaniExportSettings>();
            if (janiSettings.isJaniFileSet()) {
                storm::api::exportJaniModel(*model, {storm::jani::Property("time-bounded", tbUntil), storm::jani::Property("mttf", rewFormula)}, janiSettings.getJaniFilename());
            }

            delete model;
            delete gspn;
        }

        template<>
        void transformToGSPN(storm::storage::DFT<storm::RationalFunction> const& dft) {
            STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Transformation to GSPN not supported for this data type.");
        }

    }
}
