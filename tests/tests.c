#include "test_tokens.h"
#include "test_elements.h"

int main(void) {
    int number_failed = 0;

    SRunner* runner = srunner_create(test_tokens_suite());
    srunner_add_suite(runner, test_elements_suite());

    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
