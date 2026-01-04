#ifndef _KERNEL_TEST_H
#define _KERNEL_TEST_H

void test_all(void);

#define ASSERT_MSG(cond, msg)                                                  \
	do {                                                                   \
		if (!(cond)) {                                                 \
			kernel_test_logger("[FAILED] " msg);                   \
			return -1;                                             \
		}                                                              \
	} while (0)

#endif // _KERNEL_TEST_H !
