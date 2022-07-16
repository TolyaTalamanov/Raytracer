#include <gtest/gtest.h>

#include <sstream>

#include <raytracer/tokenizer.hpp>

TEST(TokenizerTests, EmptyStream) {
    std::stringstream ss;
    Tokenizer t(&ss);

    EXPECT_TRUE(t.IsEnd());
}

TEST(TokenizerTests, Double) {
    std::stringstream ss{"42"};
    Tokenizer t(&ss);

    auto tok = t.GetToken();

    EXPECT_TRUE(std::holds_alternative<Tokenizer::Double>(tok));
    EXPECT_EQ(42, std::get<Tokenizer::Double>(tok).val);
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