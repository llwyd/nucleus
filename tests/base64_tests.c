#include "unity.h"
#include "base64_tests.h"
#include "base64.h"

void test_BASE64_Test0(void)
{
    char * test_vector = "foo";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 3u);
    TEST_ASSERT_EQUAL(out_len, 4u);
    TEST_ASSERT_EQUAL_STRING("Zm9v", output);
}

void test_BASE64_Test1(void)
{
    char * test_vector = "foobar";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 6u);
    TEST_ASSERT_EQUAL(out_len, 8u);
    TEST_ASSERT_EQUAL_STRING("Zm9vYmFy", output);
}

void test_BASE64_Test2(void)
{
    char * test_vector = "fo";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 2u);
    TEST_ASSERT_EQUAL(out_len, 4u);
    TEST_ASSERT_EQUAL_STRING("Zm8=", output);
}

void test_BASE64_Test3(void)
{
    char * test_vector = "f";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 1u);
    TEST_ASSERT_EQUAL(out_len, 4u);
    TEST_ASSERT_EQUAL_STRING("Zg==", output);
}

void test_BASE64_Test4(void)
{
    char * test_vector = "";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 0u);
    TEST_ASSERT_EQUAL(out_len, 0);
    TEST_ASSERT_EQUAL_STRING("", output);
}

void test_BASE64_Test5(void)
{
    char * test_vector = "foob";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 4u);
    TEST_ASSERT_EQUAL(out_len, 8u);
    TEST_ASSERT_EQUAL_STRING("Zm9vYg==", output);
}

void test_BASE64_Test6(void)
{
    char * test_vector = "fooba";
    char output[32] = {0};
    uint32_t len = strlen(test_vector);
    uint32_t out_len = BASE64_Encode((uint8_t*)test_vector,len,(uint8_t*)output);

    TEST_ASSERT_EQUAL(len, 5u);
    TEST_ASSERT_EQUAL(out_len, 8u);
    TEST_ASSERT_EQUAL_STRING("Zm9vYmE=", output);
}

extern void BASE64TestSuite(void)
{
    RUN_TEST(test_BASE64_Test0);
    RUN_TEST(test_BASE64_Test1);
    RUN_TEST(test_BASE64_Test2);
    RUN_TEST(test_BASE64_Test3);
    RUN_TEST(test_BASE64_Test4);
    RUN_TEST(test_BASE64_Test5);
    RUN_TEST(test_BASE64_Test6);
}
