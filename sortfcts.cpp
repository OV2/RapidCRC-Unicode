/***************************************************************************
Copyright 2004 Sebastian Ewert

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include "resource.h"
#include "globals.h"

/*****************************************************************************
int CALLBACK SortFilename(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
lParam1		: (IN) lparam of the first list view item to be compared
lParam2		: (IN) lparam of the second list view item to be compared
lParamSort	: (IN) lParamSort. Application depending value

Return Value:
returns a value comparing the two items

Notes:
- lParamSort is from DlgProc and stores info if we have to sort ascending or
descending
- uses lstrcmpi to compare filenames
*****************************************************************************/
int CALLBACK SortFilename(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int iResult = QuickCompFunction( &lParam1, &lParam2);

	if( (*((DWORD *)lParamSort)) & SORT_FLAG_ASCENDING)
		return iResult;
	else
		return iResult * (-1);
}

/*****************************************************************************
int CALLBACK SortInfo(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
lParam1		: (IN) lparam of the first list view item to be compared
lParam2		: (IN) lparam of the second list view item to be compared
lParamSort	: (IN) lParamSort. Application depending value

Return Value:
returns a value comparing the two items

Notes:
- lParamSort is from DlgProc and stores info if we have to sort ascending or
descending
*****************************************************************************/
int CALLBACK SortInfo(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// File OK < File not OK < No CRC/MD5 found < Error

	int iResult1, iResult2;

    iResult1 = ((FILEINFO *)lParam1)->status;
    iResult2 = ((FILEINFO *)lParam2)->status;

	if( (*((DWORD *)lParamSort)) & SORT_FLAG_ASCENDING)
		return iResult1 - iResult2;
	else
		return iResult2 - iResult1;
}

/*****************************************************************************
int CALLBACK SortHash(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
lParam1		: (IN) lparam of the first list view item to be compared
lParam2		: (IN) lparam of the second list view item to be compared
lParamSort	: (IN) lParamSort. Application dependend value

Return Value:
returns a value comparing the two items

Notes:
- lParamSort is from DlgProc and stores info if we have to sort ascending or
descending
*****************************************************************************/
int CALLBACK SortHash(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int iResult;

    int hash_num = *((DWORD *)lParamSort) >> 8;
    if(hash_num == HASH_TYPE_CRC32) {
        iResult = CRCI((FILEINFO *)lParam1).r.dwCrc32Result > CRCI((FILEINFO *)lParam2).r.dwCrc32Result;
        if(iResult == 0)
            iResult = -1;
    } else
        iResult = memcmp( &((FILEINFO *)lParam1)->hashInfo[hash_num].r, &((FILEINFO *)lParam2)->hashInfo[hash_num].r, g_hash_lengths[hash_num]);
	if( (*((DWORD *)lParamSort)) & SORT_FLAG_ASCENDING)
		return iResult;
	else
		return iResult * (-1);
	return 0;
}

/*****************************************************************************
FILEINFO_STATUS InfoToIntValue(CONST FILEINFO * pFileinfo)
Fileinfo	: (IN) item with the needed info

Return Value:
returns a generated integer value

Notes:
- generated integer is used in MySortInfo to compare items
*****************************************************************************/
FILEINFO_STATUS InfoToIntValue(CONST FILEINFO * pFileinfo)
{
	// File OK < File not OK < No CRC/MD5 found < Error
	FILEINFO_STATUS iResult;
    UINT uiOkCount = 0, uiNotOkCount = 0, uiFilename = 0, uiStream = 0;

	if(pFileinfo->dwError != NO_ERROR)
		iResult = STATUS_ERROR;
	else {
        for(int i=0; i < NUM_HASH_TYPES; i++) {
            if(pFileinfo->hashInfo[i].dwFound) {
                if( (pFileinfo->parentList->bCalculated[i]) ){
                    bool bOk = false;
                    if(i == HASH_TYPE_CRC32)
                        bOk = (CRCI(pFileinfo).r.dwCrc32Result == CRCI(pFileinfo).f.dwCrc32Found);
                    else
                        bOk = (memcmp( &pFileinfo->hashInfo[i].r,
                                   &pFileinfo->hashInfo[i].f,
                                   g_hash_lengths[i] ) == 0);
                    if(bOk)
				        uiOkCount++;
			        else
				        uiNotOkCount++;
                } else {
                    if(pFileinfo->hashInfo[i].dwFound == HASH_FOUND_FILENAME)
                        uiFilename++;
                    else
                        uiStream++;
                }
		    }
        }
        if(uiNotOkCount)
            iResult = STATUS_NOT_OK;
        else if(uiOkCount)
            iResult = STATUS_OK;
        else if(uiFilename)
            iResult = STATUS_HASH_FILENAME;
        else if(uiStream)
            iResult = STATUS_HASH_STREAM;
        else
            iResult = STATUS_NO_CRC;
	}

	return iResult;
}

bool ListCompFunction(const FILEINFO& pFileinfo1, const FILEINFO& pFileinfo2)
{
	TCHAR szFilenameTemp1[MAX_PATH_EX];
	TCHAR szFilenameTemp2[MAX_PATH_EX];
	INT iDiff;

	StringCchCopy(szFilenameTemp1, MAX_PATH_EX, pFileinfo1.szFilenameShort);
	StringCchCopy(szFilenameTemp2, MAX_PATH_EX, pFileinfo2.szFilenameShort);

	ReduceToPath(szFilenameTemp1);
	ReduceToPath(szFilenameTemp2);

	iDiff = lstrcmpi( szFilenameTemp1, szFilenameTemp2);
	if(iDiff==0)
		iDiff = lstrcmpi( pFileinfo1.szFilenameShort, pFileinfo2.szFilenameShort);

	if(iDiff < 0)
		return true;
	else
		return false;
}

bool ListPointerCompFunction(const FILEINFO *pFileinfo1, const FILEINFO *pFileinfo2)
{
	return ListCompFunction(*pFileinfo1,*pFileinfo2);
}

bool ListPointerUniqFunction(const FILEINFO *pFileinfo1, const FILEINFO *pFileinfo2)
{
	return (lstrcmp(pFileinfo1->szFilename,pFileinfo2->szFilename)==0);
}

/*****************************************************************************
VOID QuickSortList()

Return Value:
- returns nothing
*****************************************************************************/
VOID QuickSortList(lFILEINFO *fileList)
{
	fileList->fInfos.sort(ListCompFunction);

	return;
}

/*****************************************************************************
INT QuickCompFunction(const void * pElem1, const void * pElem2)

Return Value:
- returns a comparison value to signal which element is smaller

Notes:
- compare function for MS qsort
- if you have an array 'int test[5]' then test has the type int * . If qsort sorts
this array both elements that are to be to compared are to of type int *, too.
That means qsort references the items as test + i
*****************************************************************************/
INT QuickCompFunction(const void * pElem1, const void * pElem2)
{
	CONST FILEINFO * CONST pFileinfo1 = *(FILEINFO **)pElem1;
	CONST FILEINFO * CONST pFileinfo2 = *(FILEINFO **)pElem2;
	TCHAR szFilenameTemp1[MAX_PATH_EX];
	TCHAR szFilenameTemp2[MAX_PATH_EX];
	INT iDiff;

	StringCchCopy(szFilenameTemp1, MAX_PATH_EX, pFileinfo1->szFilenameShort);
	StringCchCopy(szFilenameTemp2, MAX_PATH_EX, pFileinfo2->szFilenameShort);

	ReduceToPath(szFilenameTemp1);
	ReduceToPath(szFilenameTemp2);

	iDiff = lstrcmpi( szFilenameTemp1, szFilenameTemp2);

	if(iDiff != 0)
		return iDiff;
	else
		return lstrcmpi( pFileinfo1->szFilenameShort, pFileinfo2->szFilenameShort);
}
