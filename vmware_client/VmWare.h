#pragma once

typedef unsigned __int64 VM_ADDR;

struct vm_map_structure
{
	char fill[0x60];

	VM_ADDR unk;
	VM_ADDR unk2;
	VM_ADDR unk3;
	VM_ADDR AllocationPageBase;
};

struct VCPUStructure
{
	char fill[3000];
};

extern bool	(__fastcall * Vm_ValidatePhysicalAddr)	(VM_ADDR Address, VM_ADDR Size);
extern PVOID	(__fastcall * Vm_MapPhysicalAddr)		(VM_ADDR Address, VM_ADDR Size, int Flags, vm_map_structure *Unk);
extern void	(__fastcall * Vm_UnmapPhysicalAddr)		(vm_map_structure *Unk);

extern VCPUStructure	*(__fastcall * GetVCPUStruct)(int CPUIndex, const char *Identifier, int unk2);

bool Vm_ReadMemory(ULONG_PTR PhysicalAddress, PVOID Buffer, ULONG_PTR Size);
bool Vm_WriteMemory(ULONG_PTR PhysicalAddress, PVOID Buffer, ULONG_PTR Size);