/*
* Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
*                    Shengyuan Liu <sounder.liu@gmail.com>
*
* This file is part of libsquash, distributed under the MIT License
* For full terms see the included LICENSE file
*/

#ifndef ENCLOSE_IO_WINAPI_H_VD6Y96RT
#define ENCLOSE_IO_WINAPI_H_VD6Y96RT

#ifndef STATUS_NO_MORE_FILES
# define STATUS_NO_MORE_FILES ((NTSTATUS) 0x80000006L)
#endif

#ifndef STATUS_NOT_IMPLEMENTED
# define STATUS_NOT_IMPLEMENTED ((NTSTATUS) 0xC0000002L)
#endif

#ifndef STATUS_SUCCESS
# define STATUS_SUCCESS ((NTSTATUS) 0x00000000L)
#endif

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef VOID (NTAPI *PIO_APC_ROUTINE)
             (PVOID ApcContext,
              PIO_STATUS_BLOCK IoStatusBlock,
              ULONG Reserved);

typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation = 1,
	FileFullDirectoryInformation,
	FileBothDirectoryInformation,
	FileBasicInformation,
	FileStandardInformation,
	FileInternalInformation,
	FileEaInformation,
	FileAccessInformation,
	FileNameInformation,
	FileRenameInformation,
	FileLinkInformation,
	FileNamesInformation,
	FileDispositionInformation,
	FilePositionInformation,
	FileFullEaInformation,
	FileModeInformation,
	FileAlignmentInformation,
	FileAllInformation,
	FileAllocationInformation,
	FileEndOfFileInformation,
	FileAlternateNameInformation,
	FileStreamInformation,
	FilePipeInformation,
	FilePipeLocalInformation,
	FilePipeRemoteInformation,
	FileMailslotQueryInformation,
	FileMailslotSetInformation,
	FileCompressionInformation,
	FileObjectIdInformation,
	FileCompletionInformation,
	FileMoveClusterInformation,
	FileQuotaInformation,
	FileReparsePointInformation,
	FileNetworkOpenInformation,
	FileAttributeTagInformation,
	FileTrackingInformation,
	FileIdBothDirectoryInformation,
	FileIdFullDirectoryInformation,
	FileValidDataLengthInformation,
	FileShortNameInformation,
	FileIoCompletionNotificationInformation,
	FileIoStatusBlockRangeInformation,
	FileIoPriorityHintInformation,
	FileSfioReserveInformation,
	FileSfioVolumeInformation,
	FileHardLinkInformation,
	FileProcessIdsUsingFileInformation,
	FileNormalizedNameInformation,
	FileNetworkPhysicalNameInformation,
	FileIdGlobalTxDirectoryInformation,
	FileIsRemoteDeviceInformation,
	FileAttributeCacheInformation,
	FileNumaNodeInformation,
	FileStandardLinkInformation,
	FileRemoteProtocolInformation,
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

/* from winternl.h */
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef NTSTATUS(NTAPI *sNtQueryDirectoryFile)
(HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN ReturnSingleEntry,
	PUNICODE_STRING FileName,
	BOOLEAN RestartScan
	);

typedef struct _FILE_DIRECTORY_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef NTSTATUS(NTAPI *sNtQueryInformationFile)
(HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass);

typedef enum _FS_INFORMATION_CLASS {
	FileFsVolumeInformation = 1,
	FileFsLabelInformation = 2,
	FileFsSizeInformation = 3,
	FileFsDeviceInformation = 4,
	FileFsAttributeInformation = 5,
	FileFsControlInformation = 6,
	FileFsFullSizeInformation = 7,
	FileFsObjectIdInformation = 8,
	FileFsDriverPathInformation = 9,
	FileFsVolumeFlagsInformation = 10,
	FileFsSectorSizeInformation = 11
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

typedef NTSTATUS(NTAPI *sNtQueryVolumeInformationFile)
(HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FsInformation,
	ULONG Length,
	FS_INFORMATION_CLASS FsInformationClass);

typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	DWORD FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
	LARGE_INTEGER AllocationSize;
	LARGE_INTEGER EndOfFile;
	ULONG         NumberOfLinks;
	BOOLEAN       DeletePending;
	BOOLEAN       Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION {
	LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
	ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
	ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {
	LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
	ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {
	ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION {
	FILE_BASIC_INFORMATION     BasicInformation;
	FILE_STANDARD_INFORMATION  StandardInformation;
	FILE_INTERNAL_INFORMATION  InternalInformation;
	FILE_EA_INFORMATION        EaInformation;
	FILE_ACCESS_INFORMATION    AccessInformation;
	FILE_POSITION_INFORMATION  PositionInformation;
	FILE_MODE_INFORMATION      ModeInformation;
	FILE_ALIGNMENT_INFORMATION AlignmentInformation;
	FILE_NAME_INFORMATION      NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

extern sNtQueryDirectoryFile pNtQueryDirectoryFile;
extern sNtQueryInformationFile pNtQueryInformationFile;
extern sNtQueryVolumeInformationFile pNtQueryVolumeInformationFile;

typedef struct _REPARSE_DATA_BUFFER {
        ULONG  ReparseTag;
        USHORT ReparseDataLength;
        USHORT Reserved;
        union {
                struct {
                        USHORT SubstituteNameOffset;
                        USHORT SubstituteNameLength;
                        USHORT PrintNameOffset;
                        USHORT PrintNameLength;
                        ULONG Flags;
                        WCHAR PathBuffer[1];
                } SymbolicLinkReparseBuffer;
                struct {
                        USHORT SubstituteNameOffset;
                        USHORT SubstituteNameLength;
                        USHORT PrintNameOffset;
                        USHORT PrintNameLength;
                        WCHAR PathBuffer[1];
                } MountPointReparseBuffer;
                struct {
                        UCHAR  DataBuffer[1];
                } GenericReparseBuffer;
        };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#endif
