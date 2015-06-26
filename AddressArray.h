#ifndef __ADDRESSARRAY_H_
#define __ADDRESSARRAY_H_

#include <windows.h>

class AddressArray
{
	public:
		AddressArray();
		~AddressArray();

		void AppendAddress(DWORD dwAddress);
		DWORD Index(DWORD index);

		DWORD GetResultCount() { return CurrentCount; }

	private:
		DWORD *dwArrayBase;
		DWORD CurrentCount;
		DWORD ArrayMax;
};

AddressArray::AddressArray()
{
	dwArrayBase = new DWORD[100];
	ArrayMax = 100;
	CurrentCount = 0;
}

AddressArray::~AddressArray()
{
	delete[] dwArrayBase;
}

void AddressArray::AppendAddress(DWORD dwAddress)
{
	dwArrayBase[CurrentCount++] = dwAddress;
	if( CurrentCount == ArrayMax )
	{
		ArrayMax *= 2;			// Double size if we've reached the boundary.
		
		DWORD *tempArrayBase = new DWORD[ArrayMax];
		
		// Copy the array over.
		for( unsigned int i = 0; i < CurrentCount; i++ )
			tempArrayBase[i] = dwArrayBase[i];

		//memcpy(tempArrayBase, dwArrayBase, sizeof(DWORD) * CurrentCount);

		delete[] dwArrayBase;

		dwArrayBase = tempArrayBase;
	}
}

DWORD AddressArray::Index(DWORD index)
{
	if( index < CurrentCount )
		return dwArrayBase[index];
	else
		return -1;
}

#endif