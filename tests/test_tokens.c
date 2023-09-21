#include <stdlib.h>

#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "../src/tokens.h"

// START_TEST(test_valid_space) {
//     char const* source = "   \n\t/*abc*/  defg";
//     struct token token;
//     enum token_status result = get_space_token(&token, source, 0);

//     ck_assert_int_eq(result, token_found);
//     ck_assert_int_eq(token.type, token_space);
//     ck_assert_int_eq(token.length, 5);
//     ck_assert_ptr_eq(token.pointer, &source[0]);
// }
// END_TEST

Test(comment, valid) {
    char const* source = "/*abc*/  defg";
    struct token token;
    struct lexical_error error;
    enum token_status result = get_comment_token(&token, source, 0, &error);

    cr_assert(eq(int, result, token_found));
    cr_assert(eq(int, token.type, token_comment));
    cr_assert(eq(int, token.length, 7));
    cr_assert(token.pointer == &source[0]);
}

// START_TEST(test_absent_comment) {
//     char const* source = "hijklm /*abc*/  defg";
//     struct token token;
//     struct lexical_error error;
//     enum token_status result = get_comment_token(&token, source, 0, &error);

//     ck_assert_int_eq(result, token_not_found);
// }
// END_TEST

// START_TEST(test_unclosed_comment) {
//     char const* source = "/*abc defg";
//     struct token token;
//     struct lexical_error error;
//     enum token_status result = get_comment_token(&token, source, 0, &error);

//     ck_assert_int_eq(result, token_error);
//     ck_assert_ptr_eq(error.source, source);
//     ck_assert_int_eq(error.pos, 0);
//     ck_assert_int_eq(error.length, 2);
// }
// END_TEST

// START_TEST(test_parse_tokens_1) {
//     char const* source = "
//         /* abc comment */
//         selector, another_selector {
//             property: value;
//         }
//     ";
//     struct token* first_token;
//     size_t tokens_number;
//     struct lexical_error error;
//     enum token_status result = parse_tokens(&first_token, &tokens_number, source, &error);

//     ck_assert_int_eq(result, token_found);
//     ck_assert_int_eq(tokens_number, 18);
// }
// END_TEST

// Suite* tokens_suite(void) {
//     Suite* suite = suite_create("Tokens");
//     TCase* space_test = tcase_create("Space");
//     TCase* comment_test = tcase_create("Comment");
//     TCase* parse_test = tcase_create("Parse tokens");

//     tcase_add_test(space_test, test_valid_space);

//     tcase_add_test(comment_test, test_valid_comment);
//     tcase_add_test(comment_test, test_absent_comment);
//     tcase_add_test(comment_test, test_unclosed_comment);

//     tcase_add_test(parse_test, test_parse_tokens_1);

//     suite_add_tcase(suite, space_test);
//     suite_add_tcase(suite, comment_test);
//     suite_add_tcase(suite, parse_test);

//     return suite;
// }

// int main(void) {
//     int number_failed;
//     Suite* suites[] = {
//         tokens_suite(),
//         0
//     };
//     Suite** suite = &suites[0];
//     SRunner* sr = srunner_create(*suite);
//     suite++;
//     while (*suite != 0) {
//         srunner_add_suite(sr, *suite);
//         suite++;
//     }

//     srunner_run_all(sr, CK_NORMAL);
//     number_failed = srunner_ntests_failed(sr);
//     srunner_free(sr);
//     return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
// }