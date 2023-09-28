#include "test_tokens.h"
#include "test_elements.h"
#include <check.h>

int main(void) {
    int number_failed = 0;

    SRunner* runner = srunner_create(test_tokens_suite());
    srunner_add_suite(runner, get_elements_test_suite());

    srunner_set_fork_status(runner, CK_NOFORK);

    srunner_run_all(runner, CK_VERBOSE);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
