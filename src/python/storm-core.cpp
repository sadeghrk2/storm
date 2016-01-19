#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "../utility/storm.h"
#include "../logic/Formulas.h"

#include <type_traits>

namespace boost {
    template<class T> T* get_pointer(std::shared_ptr<T> p) { return p.get(); }
}

namespace boost { namespace python { namespace converter {

            template <class T>
            PyObject* shared_ptr_to_python(std::shared_ptr<T> const& x)
            {
                if (!x)
                    return python::detail::none();
                else if (shared_ptr_deleter* d = std::get_deleter<shared_ptr_deleter>(x))
                    return incref( d->owner.get() );
                else
                    return converter::registered<std::shared_ptr<T> const&>::converters.to_python(&x);
            }

    /// @brief Adapter a non-member function that returns a unique_ptr to
    ///        a python function object that returns a raw pointer but
    ///        explicitly passes ownership to Python.
    template<typename T, typename ...Args>
    object adapt_unique(std::unique_ptr<T> (*fn)(Args...))
    {
        return make_function(
            [fn](Args... args) { return fn(args...).release(); },
            return_value_policy<manage_new_object>(),
            boost::mpl::vector<T*, Args...>()
        );
    }

    /// @brief Adapter a member function that returns a unique_ptr to
    ///        a python function object that returns a raw pointer but
    ///        explicitly passes ownership to Python.
    template<typename T, typename C, typename ...Args>
    object adapt_unique(std::unique_ptr<T> (C::*fn)(Args...))
    {
        return make_function(
            [fn](C& self, Args... args) { return (self.*fn)(args...).release(); },
            python::return_value_policy<manage_new_object>(),
            boost::mpl::vector<T*, C&, Args...>()
        );
    }

        }}} // namespace boost::python::converter


template<typename Source, typename Target>
void shared_ptr_implicitly_convertible() {
    boost::python::implicitly_convertible<std::shared_ptr<Source>, std::shared_ptr<Target>>();
}


template<typename T>
void register_shared_ptr() {
    boost::python::register_ptr_to_python<std::shared_ptr<T>>();
}



std::shared_ptr<storm::models::ModelBase> buildModel(storm::prism::Program const& program, std::shared_ptr<storm::logic::Formula> const& formula) {
    return storm::buildSymbolicModel<storm::RationalFunction>(program, std::vector<std::shared_ptr<storm::logic::Formula>>(1,formula)).model;
}

void printResult(std::shared_ptr<storm::modelchecker::CheckResult> result) {
    result->writeToStream(std::cout);
}

void setupStormLib(std::string const& args) {
    storm::utility::setUp();
    storm::settings::SettingsManager::manager().setFromString(args);
}

// primary class template
template<typename S, typename T, typename Enable = void>
struct ImplConversionSharedPtr
{
    void c() { shared_ptr_implicitly_convertible<S,T>(); }
};

// specialized class template
template<typename S, typename T>
struct ImplConversionSharedPtr<S, T, typename std::enable_if<std::is_same<T, void>::value>::type>
{
    void c() { }
};

template<typename B>
struct bases_holder {
    typedef boost::python::bases<B> Type;
};

template<>
struct bases_holder<void> {
    typedef boost::python::bases<> Type;
};


template<typename C, typename B=void, typename NC=void>
boost::python::class_<C, std::shared_ptr<C>, typename bases_holder<B>::Type, NC> defineClass(char const* name, char const* docstring, typename  std::enable_if_t<std::is_default_constructible<C>::value>::type* = 0) {
    auto inst = boost::python::class_<C, std::shared_ptr<C>, typename bases_holder<B>::Type, NC>(name, docstring);
    register_shared_ptr<C>();
    ImplConversionSharedPtr<C,B>().c();
    return inst;
};

template<typename C, typename B=void, typename NC=void>
boost::python::class_<C, std::shared_ptr<C>, typename bases_holder<B>::Type, NC> defineClass(char const* name, char const* docstring) {
    auto inst = boost::python::class_<C, std::shared_ptr<C>, typename bases_holder<B>::Type, NC>(name, docstring, boost::python::no_init);
    register_shared_ptr<C>();
    ImplConversionSharedPtr<C,B>().c();
    return inst;
};


BOOST_PYTHON_MODULE(_core)
{
    using namespace boost::python;
    def("setUp", setupStormLib);

    ////////////////////////////////////////////
    // Formula
    ////////////////////////////////////////////
    class_<storm::logic::Formula, std::shared_ptr<storm::logic::Formula>, boost::noncopyable>("Formula", no_init)
        .def("__str__", &storm::logic::Formula::toString);
	class_<std::vector<std::shared_ptr<storm::logic::Formula>>>("FormulaVec")
			.def(vector_indexing_suite<std::vector<std::shared_ptr<storm::logic::Formula>>, true>())
			;
    class_<storm::logic::ProbabilityOperatorFormula, std::shared_ptr<storm::logic::ProbabilityOperatorFormula>, bases<storm::logic::Formula>>("ProbabilityOperatorFormula", no_init)
        .def("__str__", &storm::logic::ProbabilityOperatorFormula::toString);

    register_ptr_to_python<std::shared_ptr<storm::logic::Formula>>();

    ////////////////////////////////////////////
    // Program
    ////////////////////////////////////////////

    defineClass<storm::prism::Program>("Program", "")
    //class_<storm::prism::Program>("Program")
        .add_property("nr_modules", &storm::prism::Program::getNumberOfModules)
    ;


    ////////////////////////////////////////////
    // Checkresult
    ////////////////////////////////////////////
    class_<storm::modelchecker::CheckResult, std::unique_ptr<storm::modelchecker::CheckResult>, boost::noncopyable>("CheckResult", no_init)
    ;
    register_ptr_to_python<std::shared_ptr<storm::modelchecker::CheckResult>>();

    def("print_result", printResult);


    ////////////////////////////////////////////
    // Models
    ////////////////////////////////////////////

    enum_<storm::models::ModelType>("ModelType")
            .value("dtmc", storm::models::ModelType::Dtmc)
            .value("mdp", storm::models::ModelType::Mdp)
            .value("ctmc", storm::models::ModelType::Ctmc)
            .value("ma", storm::models::ModelType::MarkovAutomaton)
            ;

    defineClass<storm::models::ModelBase, void, boost::noncopyable>("ModelBase", "")
         .add_property("nr_states", &storm::models::ModelBase::getNumberOfStates)
            .add_property("nr_transitions", &storm::models::ModelBase::getNumberOfTransitions)
            .add_property("model_type", &storm::models::ModelBase::getType)
            .def("asPdtmc", &storm::models::ModelBase::as<storm::models::sparse::Dtmc<storm::RationalFunction>>)
            .def("asPmdp", &storm::models::ModelBase::as<storm::models::sparse::Mdp<storm::RationalFunction>>)
    ;

    defineClass<storm::models::sparse::Model<storm::RationalFunction>, storm::models::ModelBase, boost::noncopyable>("SparseParametricModel", "");
    //class_<storm::models::sparse::Model<storm::RationalFunction>, std::shared_ptr<storm::models::sparse::Model<storm::RationalFunction> >, boost::noncopyable, bases<storm::models::ModelBase>>("SparseParametricModel", no_init);
    //register_shared_ptr<storm::models::sparse::Model<storm::RationalFunction>>();
    //shared_ptr_implicitly_convertible<storm::models::sparse::Model<storm::RationalFunction>, storm::models::ModelBase>();

    class_<storm::models::sparse::Model<double>, std::shared_ptr<storm::models::sparse::Model<double>>, boost::noncopyable, bases<storm::models::ModelBase>>("SparseModel", no_init);
    register_shared_ptr<storm::models::sparse::Model<double>>();
    shared_ptr_implicitly_convertible<storm::models::sparse::Dtmc<double>, storm::models::sparse::Model<double>>();

    defineClass<storm::models::sparse::Dtmc<storm::RationalFunction>,  storm::models::sparse::Model<storm::RationalFunction>>("SparseParametricMc", "");
    //class_<storm::models::sparse::Dtmc<storm::RationalFunction>, std::shared_ptr<storm::models::sparse::Dtmc<storm::RationalFunction>>, boost::noncopyable, bases<storm::models::sparse::Model<storm::RationalFunction>>>("SparseParametricMc", no_init);
    //register_shared_ptr<storm::models::sparse::Dtmc<storm::RationalFunction>>();
    //shared_ptr_implicitly_convertible<storm::models::sparse::Dtmc<storm::RationalFunction>, storm::models::sparse::Model<storm::RationalFunction>>();

    class_<storm::models::sparse::Mdp<storm::RationalFunction>, std::shared_ptr<storm::models::sparse::Dtmc<storm::RationalFunction>>, boost::noncopyable, bases<storm::models::sparse::Model<storm::RationalFunction>>>("SparseParametricMdp", no_init);
    register_shared_ptr<storm::models::sparse::Mdp<storm::RationalFunction>>();



    def("parse_formulae", storm::parseFormulasForProgram);
    def("parse_program", storm::parseProgram);

    def("build_model", buildModel, return_value_policy<return_by_value>());

    def("buildodelFromPrismProgram", storm::buildSymbolicModel<double>);
    def("buildParametricModelFromPrismProgram", storm::buildSymbolicModel<storm::RationalFunction>);

    //////////////////////////////////////////////
    // Model Checking
    //////////////////////////////////////////////
    class_<storm::storage::ModelFormulasPair>("ModelProgramPair", no_init)
            .add_property("model", &storm::storage::ModelFormulasPair::model)
            .add_property("program", &storm::storage::ModelFormulasPair::formulas)
    ;


    def("perform_state_elimination", boost::python::converter::adapt_unique(storm::verifySparseModel<storm::RationalFunction>));
}
