#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/test.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>
#include <stdarg.h>
#include <stdio.h>

internal int test_vmm_aliasing(void);
internal int test_vmm_unmap_virt(void);
internal void kernel_test_logger(const char *format, ...);
internal int run_test(int (*test_func)(void), int *passed, int *total);

int test_all(void)
{
	kernel_test_logger("TEST: STARTING TESTS");

	int err_out = 0;
	int passed = 0;
	int total = 0;

	err_out = run_test(test_vmm_aliasing, &passed, &total);
	err_out = err_out || run_test(test_vmm_unmap_virt, &passed, &total);

	if (err_out) {
		kernel_test_logger("TEST: FAILED");
	}
	else {
		kernel_test_logger("TEST: PASSED");
	}

	kernel_test_logger("    %d/%d passed", passed, total);

	return err_out;
}

internal int run_test(int (*test_func)(void), int *passed, int *total)
{
	int err_out = test_func();
	if (err_out == 0) {
		*passed += 1;
	}
	*total += 1;

	return err_out;
}

internal int test_vmm_aliasing(void)
{
	kernel_test_logger("TEST: VMM Aliasing... ");

	page_t *phys_frame = kmalloc_page();
	if (phys_frame == NULL) {
		kernel_test_logger("failed to allcoated page");
		return 1;
	}

	uintptr_t virt_a = 0xD0000000;
	uintptr_t virt_b = 0xE0000000;

	vmm_page_map((paddr_t)phys_frame, virt_a, 3);
	vmm_page_map((paddr_t)phys_frame, virt_b, 3);

	uintptr_t *ptr_a = (uintptr_t *)virt_a;
	uintptr_t *ptr_b = (uintptr_t *)virt_b;

	*ptr_a = 0xDEADBEEF;

	if (*ptr_b == 0xDEADBEEF) {
		kernel_test_logger("[PASSED]");
		kernel_test_logger("   Writing to %x correctly updated %x",
				   virt_a, virt_b);
		vmm_page_unmap(virt_a);
		vmm_page_unmap(virt_b);
		return 0;
	}
	else {
		kernel_test_logger("[FAILED]");
		kernel_test_logger("   Expected 0xDEADBEEF but got %x", *ptr_b);
		return 1;
	}
}

internal int test_vmm_unmap_virt(void)
{
	kernel_test_logger("TEST: VMM Unmapping... ");

	void *phys_frame_ptr = kmalloc_page();
	if (phys_frame_ptr == NULL) {
		kernel_test_logger("[FAILED] failed to allocate new page");
		return 1;
	}
	paddr_t phys_addr = (paddr_t)phys_frame_ptr;

	uintptr_t virt_a = 0xD0000000;
	uintptr_t virt_b = 0xE0000000;

	vmm_page_map(phys_addr, virt_a, 3);
	vmm_page_map(phys_addr, virt_b, 3);

	volatile uint32_t *ptr_a = (uint32_t *)virt_a;
	volatile uint32_t *ptr_b = (uint32_t *)virt_b;

	*ptr_a = 0xDEADBEEF;

	if (*ptr_b != 0xDEADBEEF) {
		kernel_test_logger(
			"[FAILED] Aliasing setup failed. B does not equal A.");
		goto cleanup;
	}

	int err = vmm_page_unmap(virt_b);
	if (err) {
		kernel_test_logger("[FAILED] vmm_unmap_page returned error");
		goto cleanup;
	}

	if (*ptr_a != 0xDEADBEEF) {
		kernel_test_logger("[FAILED] Unmapping B corrupted A");
		goto cleanup;
	}

	page_t *page_table_entry_b = (page_t *)GET_PTE_PTR(virt_b);

	if (page_table_entry_b->present == 1) {
		kernel_test_logger(
			"[FAILED] virt_b PTE is still marked PRESENT");
		goto cleanup;
	}

	kernel_test_logger("[PASSED]");
	kernel_test_logger("    Virt_A remains valid: 0x%x", *ptr_a);
	kernel_test_logger("    Virt_B marked not present.");
	return 0;

cleanup:
	vmm_page_unmap(virt_a);
	vmm_page_unmap(virt_b);
	return 1;
}

internal void kernel_test_logger(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}
