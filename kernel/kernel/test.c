#include <kernel/heap.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/test.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

internal int test_vmm_aliasing(void);
internal int test_vmm_unmap_virt(void);
internal int test_sbrk(void);
internal int test_kmalloc(void);
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
	err_out = err_out || run_test(test_sbrk, &passed, &total);
	err_out = err_out || run_test(test_kmalloc, &passed, &total);

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

internal int test_kmalloc(void)
{
	kernel_test_logger("TEST: kmalloc()");

	const char *str1 = "askdjasdkjasdkj\n";
	char *my_heap_bytes = kmalloc(strlen(str1) + 1);
	strncpy(my_heap_bytes, str1, strlen(str1) + 1);
	printf("%s", my_heap_bytes);

	const char *str2 = "HIHIIHIIH\n";
	char *my_heap_bytes_2 = kmalloc(strlen(str2) + 1);
	strncpy(my_heap_bytes_2, str2, strlen(str2) + 1);
	printf("%s", my_heap_bytes_2);
	void *newptr = krealloc(my_heap_bytes, 1024);

	kfree(my_heap_bytes_2);
	kfree(newptr);

	return 0;
}

internal int test_sbrk(void)
{
	kernel_test_logger("TEST: sbrk()");

	kernel_test_logger("   [1/7] basic allocation... ");
	uint8_t *region1 = (uint8_t *)sbrk(10);
	ASSERT_MSG(region1 != (void *)-1, "sbrk(10) returned -1");
	region1[0] = 0xAA;
	region1[9] = 0xBB;
	ASSERT_MSG(region1[0] == 0xAA,
		   "memory write verification failed (start)");
	ASSERT_MSG(region1[9] == 0xBB,
		   "memory write verification failed (end)");
	kernel_test_logger("[PASSED]");

	kernel_test_logger("   [2/7] metadata edge case... ");
	size_t edge_size = PAGE_SIZE - sizeof(struct HeapBlock) + 10;
	uint8_t *region2 = (uint8_t *)sbrk(edge_size);
	ASSERT_MSG(region2 != (void *)-1, "sbrk(edge_size) returned -1");
	region2[edge_size - 1] = 0xCC;
	ASSERT_MSG(region2[edge_size - 1] == 0xCC,
		   "edge case memory access failed");
	kernel_test_logger("[PASSED]");

	kernel_test_logger("   [3/7] multi-page (3 pages)... ");
	size_t large_size = PAGE_SIZE * 3;
	uint8_t *region3 = (uint8_t *)sbrk(large_size);
	ASSERT_MSG(region3 != (void *)-1, "sbrk(large) returned -1");
	region3[0] = 0x11;
	region3[PAGE_SIZE] = 0x22;
	region3[PAGE_SIZE * 2] = 0x33;
	region3[large_size - 1] = 0x44;
	ASSERT_MSG(region3[large_size - 1] == 0x44,
		   "large allocation end access failed");
	kernel_test_logger("[PASSED]");

	kernel_test_logger("   [4/7] sbrk(0) & continuity... ");
	void *current_break = sbrk(0);
	ASSERT_MSG(current_break != (void *)-1, "sbrk(0) failed");
	void *check_break = sbrk(0);
	ASSERT_MSG(current_break == check_break, "sbrk(0) not consistent");
	void *next_alloc = sbrk(1);
	ASSERT_MSG(next_alloc != (void *)-1, "sbrk(1) failed");

	kernel_test_logger("   [5/7] align to page boundary... ");
	void *current_addr = sbrk(0);
	ASSERT_MSG(current_addr != (void *)-1, "sbrk(0) failed");
	size_t offset = (uintptr_t)current_addr % PAGE_SIZE;
	size_t padding = (offset == 0) ? 0 : (PAGE_SIZE - offset);
	if (padding > 0) {
		void *pad_res = sbrk(padding);
		ASSERT_MSG(pad_res != (void *)-1, "padding allocation failed");
	}
	void *aligned_addr = sbrk(0);
	ASSERT_MSG((uintptr_t)aligned_addr % PAGE_SIZE == 0,
		   "heap not page aligned");

	kernel_test_logger("   [6/7] negative sbrk (page aligned)... ");
	uint8_t *start_break = (uint8_t *)sbrk(0);
	ASSERT_MSG(start_break != (void *)-1, "sbrk(0) failed");
	void *alloc_ret = sbrk(PAGE_SIZE);
	ASSERT_MSG(alloc_ret != (void *)-1, "sbrk(PAGE_SIZE) failed");
	uint8_t *end_break = (uint8_t *)sbrk(0);
	intptr_t actual_growth = end_break - start_break;
	ASSERT_MSG(actual_growth == (PAGE_SIZE),
		   "growth was not 1 page as expected");
	intptr_t shrink_val = -((intptr_t)(end_break - start_break));
	void *shrink_ret = sbrk(shrink_val);
	ASSERT_MSG(
		shrink_ret == (void *)end_break,
		"sbrk negative return value mismatch (must be previous break)");
	void *final_break = sbrk(0);
	ASSERT_MSG(final_break == start_break,
		   "heap break did not return to original position");

	kernel_test_logger("   [7/7] negative sbrk (underflow)... ");
	intptr_t huge_negative = -(1024 * 1024 * 1024);
	void *underflow_ret = sbrk(huge_negative);
	ASSERT_MSG(underflow_ret == (void *)-1,
		   "sbrk(huge_negative) should fail");
	void *stable_break = sbrk(0);
	ASSERT_MSG(stable_break == alloc_ret,
		   "break pointer moved after failed underflow");

	kernel_test_logger("[PASSED]");

	return 0;
}

internal int test_vmm_aliasing(void)
{
	kernel_test_logger("TEST: VMM Aliasing... ");

	Page *phys_frame = pmm_alloc_page();
	ASSERT_MSG(phys_frame != NULL, "failed to allocate page");

	uintptr_t virt_a = 0xD0000000;
	uintptr_t virt_b = 0xE0000000;

	vmm_page_map((paddr_t)phys_frame, virt_a, 3);
	vmm_page_map((paddr_t)phys_frame, virt_b, 3);

	uintptr_t *ptr_a = (uintptr_t *)virt_a;
	uintptr_t *ptr_b = (uintptr_t *)virt_b;

	*ptr_a = 0xDEADBEEF;

	ASSERT_MSG(*ptr_b == 0xDEADBEEF, "expected 0xdeadbeef but got %x");
	kernel_test_logger("[PASSED]");
	vmm_page_unmap(*ptr_b);
	vmm_page_unmap(*ptr_a);

	return 0;
}

internal int test_vmm_unmap_virt(void)
{
	kernel_test_logger("TEST: VMM Unmapping... ");

	void *phys_frame_ptr = pmm_alloc_page();
	ASSERT_MSG(phys_frame_ptr != NULL, "failed to allocate new page");

	paddr_t phys_addr = (paddr_t)phys_frame_ptr;

	uintptr_t virt_a = 0xD0000000;
	uintptr_t virt_b = 0xE0000000;

	vmm_page_map(phys_addr, virt_a, 3);
	vmm_page_map(phys_addr, virt_b, 3);

	volatile uint32_t *ptr_a = (uint32_t *)virt_a;
	volatile uint32_t *ptr_b = (uint32_t *)virt_b;

	*ptr_a = 0xDEADBEEF;

	ASSERT_MSG(*ptr_b == 0xDEADBEEF,
		   "Aliasing setup failed. B does not equal A.");

	int err = vmm_page_unmap(virt_b);
	ASSERT_MSG(!err, "vmm_unmap_page returned error");
	ASSERT_MSG(*ptr_a == 0xDEADBEEF, "Unmapping B corrupted A");

	volatile Page *page_table_entry_b =
		(volatile Page *)GET_PTE_PTR(virt_b);
	ASSERT_MSG(page_table_entry_b->present != 1,
		   "virt_b PTE is still marked PRESENT");

	kernel_test_logger("[PASSED]");
	return 0;
}

internal void kernel_test_logger(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}
