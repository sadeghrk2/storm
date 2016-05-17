#include "gtest/gtest.h"
#include "storm-config.h"
#include "src/parser/FormulaParser.h"
#include "src/logic/FragmentSpecification.h"
#include "src/exceptions/WrongFormatException.h"

TEST(FormulaParserTest, LabelTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "\"label\"";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
	ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isAtomicLabelFormula());
}

TEST(FormulaParserTest, ComplexLabelTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "!(\"a\" & \"b\") | \"a\" & !\"c\"";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
	ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
    EXPECT_TRUE(formula->isBinaryBooleanStateFormula());
}

TEST(FormulaParserTest, ExpressionTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");
    
    storm::parser::FormulaParser formulaParser(manager);
    
    std::string input = "!(x | y > 3)";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    
    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
    EXPECT_TRUE(formula->isUnaryBooleanStateFormula());
}

TEST(FormulaParserTest, LabelAndExpressionTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");
    
    storm::parser::FormulaParser formulaParser(manager);
    
    std::string input = "!\"a\" | x | y > 3";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    
    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
    
    input = "x | y > 3 | !\"a\"";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isInFragment(storm::logic::propositional()));
}

TEST(FormulaParserTest, ProbabilityOperatorTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "P<0.9 [\"a\" U \"b\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
	ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));

    EXPECT_TRUE(formula->isProbabilityOperatorFormula());
    EXPECT_TRUE(formula->asProbabilityOperatorFormula().hasBound());
    EXPECT_FALSE(formula->asProbabilityOperatorFormula().hasOptimalityType());
}

TEST(FormulaParserTest, RewardOperatorTest) {
    storm::parser::FormulaParser formulaParser;
    
    std::string input = "Rmin<0.9 [F \"a\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    
    EXPECT_TRUE(formula->isRewardOperatorFormula());
    EXPECT_TRUE(formula->asRewardOperatorFormula().hasBound());
    EXPECT_TRUE(formula->asRewardOperatorFormula().hasOptimalityType());
    
    input = "R=? [I=10]";
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    
    EXPECT_TRUE(formula->isRewardOperatorFormula());
    EXPECT_FALSE(formula->asRewardOperatorFormula().hasBound());
    EXPECT_FALSE(formula->asRewardOperatorFormula().hasOptimalityType());
    EXPECT_TRUE(formula->asRewardOperatorFormula().getSubformula().isInstantaneousRewardFormula());
}

TEST(FormulaParserTest, ConditionalProbabilityTest) {
    storm::parser::FormulaParser formulaParser;
    
    std::string input = "P<0.9 [F \"a\" || F \"b\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    
    EXPECT_TRUE(formula->isProbabilityOperatorFormula());
    storm::logic::ProbabilityOperatorFormula const& probFormula = formula->asProbabilityOperatorFormula();
    EXPECT_TRUE(probFormula.getSubformula().isConditionalProbabilityFormula());
}

TEST(FormulaParserTest, NestedPathFormulaTest) {
    storm::parser::FormulaParser formulaParser;
    
    std::string input = "P<0.9 [F X \"a\"]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
    ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    
    EXPECT_TRUE(formula->isProbabilityOperatorFormula());
    ASSERT_TRUE(formula->asProbabilityOperatorFormula().getSubformula().isEventuallyFormula());
    ASSERT_TRUE(formula->asProbabilityOperatorFormula().getSubformula().asEventuallyFormula().getSubformula().isNextFormula());
}

TEST(FormulaParserTest, CommentTest) {
    storm::parser::FormulaParser formulaParser;

    std::string input = "// This is a comment. And this is a commented out formula: P<=0.5 [ F \"a\" ] The next line contains the actual formula. \n P<=0.5 [ X \"b\" ] // Another comment \n // And again: another comment.";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
	ASSERT_NO_THROW(formula = formulaParser.parseSingleFormulaFromString(input));
    EXPECT_TRUE(formula->isProbabilityOperatorFormula());
    ASSERT_TRUE(formula->asProbabilityOperatorFormula().getSubformula().isNextFormula());
    ASSERT_TRUE(formula->asProbabilityOperatorFormula().getSubformula().asNextFormula().getSubformula().isAtomicLabelFormula());
}


TEST(FormulaParserTest, WrongFormatTest) {
    std::shared_ptr<storm::expressions::ExpressionManager> manager(new storm::expressions::ExpressionManager());
    manager->declareBooleanVariable("x");
    manager->declareIntegerVariable("y");
    
    storm::parser::FormulaParser formulaParser(manager);
    std::string input = "P>0.5 [ a ]";
    std::shared_ptr<storm::logic::Formula const> formula(nullptr);
	EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);
    
    input = "P=0.5 [F \"a\"]";
    EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "P>0.5 [F !(x = 0)]";
    EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "P>0.5 [F !y]";
    EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);

    input = "P>0.5 [F y!=0)]";
    EXPECT_THROW(formula = formulaParser.parseSingleFormulaFromString(input), storm::exceptions::WrongFormatException);
}
