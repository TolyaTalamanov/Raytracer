#include <gtest/gtest.h>

#include <sstream>

#include <raytracer/tokenizer.hpp>

TEST(TokenizerTests, EmptyStream) {
    std::stringstream ss;
    Tokenizer t(&ss);

    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, Double) {
    std::stringstream ss{"42.0"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::Double>(tok));
    EXPECT_EQ(42, std::get<Tokenizer::Double>(tok).val);
}

TEST(TokenizerTests, Int) {
    std::stringstream ss{"42"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::Double>(tok));
    EXPECT_EQ(42, std::get<Tokenizer::Double>(tok).val);
}

TEST(TokenizerTests, Negative) {
    std::stringstream ss{"-42.5"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::Double>(tok));
    EXPECT_EQ(-42.5, std::get<Tokenizer::Double>(tok).val);
}

TEST(TokenizerTests, StringWithDot) {
    std::stringstream ss{"42.0.0"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::String>(tok));
    EXPECT_EQ("42.0.0", std::get<Tokenizer::String>(tok).str);
}

TEST(TokenizerTests, StringStartsWithNumber) {
    std::stringstream ss{"1212_foo"};
    Tokenizer t(&ss);

    EXPECT_EQ("1212_foo", std::get<Tokenizer::String>(t.GetToken()).str);
}

TEST(TokenizerTests, NumbersWithSlashes) {
    std::stringstream ss{"0/1/2/"};
    Tokenizer t(&ss);

    for (int i = 0; i < 3; ++i) {
        auto tok = t.GetToken();
        ASSERT_EQ(i, std::get<Tokenizer::Double>(tok).val);
        t.Next();
        tok = t.GetToken();
        ASSERT_TRUE(std::holds_alternative<Tokenizer::Slash>(tok));
        t.Next();
    }

    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, String) {
    std::stringstream ss{"foo"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::String>(tok));
    EXPECT_EQ("foo", std::get<Tokenizer::String>(tok).str);
}

TEST(TokenizerTests, Slash) {
    std::stringstream ss{"/"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::Slash>(tok));
}

TEST(TokenizerTests, TokenCombination) {
    std::stringstream ss{"4\n/foo"};
    Tokenizer t(&ss);

    EXPECT_EQ(4, std::get<Tokenizer::Double>(t.GetToken()).val);
    t.Next();
    EXPECT_TRUE(std::holds_alternative<Tokenizer::Slash>(t.GetToken()));
    t.Next();
    EXPECT_EQ("foo", std::get<Tokenizer::String>(t.GetToken()).str);
    t.Next();
    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, GetTokenReturnTheSame) {
    std::stringstream ss{"42"};
    Tokenizer t(&ss);

    EXPECT_EQ(42, std::get<Tokenizer::Double>(t.GetToken()).val);
    EXPECT_EQ(42, std::get<Tokenizer::Double>(t.GetToken()).val);
}

TEST(TokenizerTests, GetTokenDoesNotMoveCursor) {
    std::stringstream ss{"42"};
    Tokenizer t(&ss);

    (void)t.GetToken();
    EXPECT_FALSE(t.IsEnd());
    t.Next();
    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, SkipCommentsBegin) {
    std::stringstream ss{"# foo\n42"};
    Tokenizer t(&ss);

    EXPECT_EQ(42, std::get<Tokenizer::Double>(t.GetToken()).val);
    t.Next();
    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, SkipCommentsMiddle) {
    std::stringstream ss{"/# foo\n42"};
    Tokenizer t(&ss);

    EXPECT_TRUE(std::holds_alternative<Tokenizer::Slash>(t.GetToken()));
    t.Next();
    EXPECT_EQ(42, std::get<Tokenizer::Double>(t.GetToken()).val);
    t.Next();
    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, SkipToken) {
    std::stringstream ss{"foo 42"};
    Tokenizer t(&ss);

    t.Next();

    auto tok = t.GetToken();
    EXPECT_TRUE(std::holds_alternative<Tokenizer::Double>(tok));
    EXPECT_EQ(42, std::get<Tokenizer::Double>(tok).val);
}

TEST(TokenizerTests, NoThrowGetTokenAfterEnd) {
    std::stringstream ss{"foo"};
    Tokenizer t(&ss);

    t.GetToken();
    t.Next();

    ASSERT_NO_THROW(t.GetToken());
    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, NextLineBeforeEnd) {
    std::stringstream ss{"#foo"};
    Tokenizer t(&ss);

    t.NextLine();

    EXPECT_TRUE(t.IsEnd());
}
