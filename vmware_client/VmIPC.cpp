#include "stdafx.h"

HANDLE g_Pipe;
HANDLE g_MapFile;
LPVOID g_MapMemory;

bool VmEstablishIPC()
{
	// Establish a pipe for reading and writing messages
	g_Pipe = CreateNamedPipe(VM_CLIENT_IPC_PATH, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, sizeof(IPCPacket), sizeof(IPCPacket), 0, nullptr);

	DWORD Err = GetLastError();
	if (g_Pipe == INVALID_HANDLE_VALUE)
		return false;

	// Create a mapping of the page files
	g_MapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, VM_CLIENT_IPC_MEM_SIZE, VM_CLIENT_IPC_MEM_PATH);

	if (!g_MapFile)
		return false;

	// Now map it into the process's memory
	g_MapMemory = MapViewOfFile(g_MapFile, FILE_MAP_ALL_ACCESS, 0, 0, VM_CLIENT_IPC_MEM_SIZE);

	if (!g_MapMemory)
		return false;

	// Start the monitoring thread
	HANDLE hThread = CreateThread(nullptr, 0, PipeThreadMonitor, nullptr, 0, nullptr);

	CloseHandle(hThread);

	return true;
}

void VmTerminateIPC()
{
	if (g_MapMemory)
		UnmapViewOfFile(g_MapMemory);

	if (g_Pipe)
		CloseHandle(g_Pipe);

	if (g_MapFile)
		CloseHandle(g_MapFile);

	g_MapMemory = 0;
	g_Pipe		= 0;
	g_MapFile	= 0;
}

bool ReadFromPipe(IPCPacket *Packet)
{
	DWORD bytesRead;

	if (!ReadFile(g_Pipe, Packet, sizeof(IPCPacket), &bytesRead, nullptr))
		return false;

	return bytesRead == sizeof(IPCPacket);
}

bool WriteToPipe(IPCPacket *Packet)
{
	DWORD bytesWritten;

	if (!WriteFile(g_Pipe, Packet, sizeof(IPCPacket), &bytesWritten, nullptr))
		return false;

	return bytesWritten == sizeof(IPCPacket);
}

void VmHandleReadMem(IPCPacket *Packet)
{
	// Parameters:
	// 0: Physical (0)/Virtual (1)
	// 1: Address
	// 2: Size
	// 3: Physical memory base

	ULONG64 type = Packet->Parameters[0];
	ULONG64 addr = Packet->Parameters[1];
	ULONG64 size = Packet->Parameters[2];
	ULONG64 base = Packet->Parameters[3];

	size = max(size, 4096);

	// Check if it was virtual memory
	if (type == 1)
		addr = AMD64_VirtualToPhysical(base, addr);

	if (!addr)
	{
		Packet->Status = 0;
		return;
	}

	if (Vm_ReadMemory(addr, g_MapMemory, size))
		Packet->Status = 1;
	else
		Packet->Status = 0;
}

void VmHandleWriteMem(IPCPacket *Packet)
{
	// Parameters:
	// 0: Physical (0)/Virtual (1)
	// 1: Address
	// 2: Size
	// 3: Physical memory base

	ULONG64 type = Packet->Parameters[0];
	ULONG64 addr = Packet->Parameters[1];
	ULONG64 size = Packet->Parameters[2];
	ULONG64 base = Packet->Parameters[3];

	size = max(size, 4096);

	// Check if it was virtual memory
	if (type == 1)
		addr = AMD64_VirtualToPhysical(base, addr);

	if (!addr)
	{
		Packet->Status = 0;
		return;
	}

	if (Vm_WriteMemory(addr, g_MapMemory, size))
		Packet->Status = 1;
	else
		Packet->Status = 0;
}

DWORD WINAPI PipeThreadMonitor(LPVOID Arg)
{
	IPCPacket packet;

	for (;;)
	{
		// Wait for the client to connect
		if (!ConnectNamedPipe(g_Pipe, nullptr))
		{
			printf("An error occurred while waiting for a client connection\n");
			return 1;
		}

		for (bool runloop = true; runloop;)
		{
			// Wait for a client message to be sent
			if (!ReadFromPipe(&packet))
			{
				printf("Failed to read from client connection\n");
				
				runloop = false;
				break;
			}

			// Handle the message here
			switch (packet.MessageType)
			{
			case VM_CLIENT_READMEM_CMD:
				VmHandleReadMem(&packet);
				break;

			case VM_CLIENT_WRITEMEM_CMD:
				VmHandleWriteMem(&packet);
				break;

			case VM_CLIENT_SHUTDOWN_CMD:
				printf("GOT A SHUTDOWN MESSAGE TYPE\n");

				// Exit the loop
				runloop = false;
				break;

			default:
				printf("GOT AN UNKNOWN MESSAGE TYPE\n");

				packet.Status = 0;
				break;
			}

			// Send the resulting packet
			if (!WriteToPipe(&packet))
				return 1;
		}

		// Disconnect the client
		DisconnectNamedPipe(g_Pipe);
	}

	return 0;
}