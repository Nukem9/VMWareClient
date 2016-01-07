#include "stdafx.h"

// Return the Page Map Level 4 Entry for the given virtual address
ULONG_PTR AMD64_PML4E(ULONG_PTR PhysBase, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:12 are from CR3
	* Bits 11 : 3 are bits 47 : 39 of the linear address
	* Bits 2 : 0 are 0.
	*
	* DTB = Directory Table Base (CR3 value)
	*/

	ULONG_PTR pml4e_addr = (PhysBase & 0xffffffffff000) | ((VirtualAddress & 0xff8000000000) >> 36);
	ULONG_PTR pml4e_val = 0;

	if (!Vm_ReadMemory(pml4e_addr, (PVOID) &pml4e_val, sizeof(ULONG_PTR)))
		return 0;

	return pml4e_val;
}

// Return the Page Directory Pointer Table Entry for the given virtual address
ULONG_PTR AMD64_PDPTE(ULONG_PTR PML4E, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:12 are from the PML4E
	* Bits 11 : 3 are bits 38 : 30 of the linear address
	* Bits 2 : 0 are all 0
	*/

	ULONG_PTR pdpte_addr = (PML4E & 0xffffffffff000) | ((VirtualAddress & 0x7fc0000000) >> 27);
	ULONG_PTR pdpte_val = 0;

	if (!Vm_ReadMemory(pdpte_addr, (PVOID) &pdpte_val, sizeof(ULONG_PTR)))
		return 0;

	return pdpte_val;
}

// Return the Page Directory Entry for the given virtual address and Page Directory Pointer Table Entry
ULONG_PTR AMD64_PDE(ULONG_PTR PDPTE, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:12 are from the PDPTE
	* Bits 11 : 3 are bits 29 : 21 of the linear address
	* Bits 2 : 0 are 0
	*/

	ULONG_PTR pde_addr = (PDPTE & 0xffffffffff000) | ((VirtualAddress & 0x3fe00000) >> 18);
	ULONG_PTR pde_val = 0;

	if (!Vm_ReadMemory(pde_addr, (PVOID) &pde_val, sizeof(ULONG_PTR)))
		return 0;

	return pde_val;
}

// Return the Page Table Entry for the given virtual address and Page Directory Entry
ULONG_PTR AMD64_PTE(ULONG_PTR PDE, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:12 are from the PDE
	* Bits 11 : 3 are bits 20 : 12 of the original linear address
	* Bits 2 : 0 are 0
	*/

	ULONG_PTR pte_addr = (PDE & 0xffffffffff000) | ((VirtualAddress & 0x1ff000) >> 9);
	ULONG_PTR pte_val = 0;

	if (!Vm_ReadMemory(pte_addr, (PVOID) &pte_val, sizeof(ULONG_PTR)))
		return 0;

	return pte_val;
}

// Return the offset in a 1GB memory page from the given virtual address and Page Directory Pointer Table Entry
ULONG_PTR AMD64_1GAddress(ULONG_PTR PDPTE, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:30 are from the PDE
	* Bits 29 : 0 are from the original linear address
	*/

	return (PDPTE & 0xfffffc0000000) | (VirtualAddress & 0x3fffffff);
}

// Return the offset in a 2MB memory page from the given virtual address and Page Directory Entry
ULONG_PTR AMD64_2MAddress(ULONG_PTR PDE, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:21 are from the PDE
	* Bits 20 : 0 are from the original linear address
	*/

	return (PDE & 0xfffffffe00000) | (VirtualAddress & 0x1fffff);
}

// Return the offset in a 4KB memory page from the given virtual address and Page Table Entry
ULONG_PTR AMD64_PhysicalAddress(ULONG_PTR PTE, ULONG_PTR VirtualAddress)
{
	/*
	* Bits 51:12 are from the PTE
	* Bits 11 : 0 are from the original linear address
	*/

	return (PTE & 0xffffffffff000) | (VirtualAddress & 0xfff);
}

// Returns whether or not the 'PS' (Page Size) flag is on in the given entry
bool AMD64_PageSizeFlag(ULONG_PTR Entry)
{
	if (Entry)
		return (Entry & (1 << 7)) == (1 << 7);

	return false;
}

ULONG_PTR AMD64_VirtualToPhysical(ULONG_PTR PhysBase, ULONG_PTR VirtualAddress)
{
	ULONG_PTR pml4e = AMD64_PML4E(PhysBase, VirtualAddress);

	// Check if it was paged out
	if (!pml4e)
		return 0;

	ULONG_PTR pdpte = AMD64_PDPTE(pml4e, VirtualAddress);

	// Check if it was paged out
	if (!pdpte)
		return 0;

	if (AMD64_PageSizeFlag(pdpte))
		return AMD64_1GAddress(pdpte, VirtualAddress);

	ULONG_PTR pde = AMD64_PDE(pdpte, VirtualAddress);

	// Check if it was paged out
	if (!pde)
		return 0;

	if (AMD64_PageSizeFlag(pde))
		return AMD64_2MAddress(pde, VirtualAddress);

	ULONG_PTR pte = AMD64_PTE(pde, VirtualAddress);

	// Check if it was paged out
	if (!pte)
		return 0;

	return AMD64_PhysicalAddress(pte, VirtualAddress);
}