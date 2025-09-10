using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Afster
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Timestamp
    {
        public ushort Year;
        public ushort Month;
        public ushort Day;
        public ushort Hours;
        public ushort Minutes;
        public ushort Seconds;

        public override string ToString()
            => $"{Day:D2}.{Month:D2}.{Year:D4} {Hours:D2}:{Minutes:D2}:{Seconds:D2}";
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct AfsEntryInfo
    {
        public uint Offset;
        public uint Size;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct AfsEntryMetadata
    {
        public const int AFSMETA_NAMEBUFFERSIZE = 0x20;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = AFSMETA_NAMEBUFFERSIZE)]
        public string FileName;
        public Timestamp LastModified;
        public uint FileSize;
    }

    internal static class AfsNative
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate IntPtr DelegateAfsOpen(string filePath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DelegateAfsFree(IntPtr afs);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate uint DelegateAfsGetEntrycount(IntPtr afs);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate IntPtr DelegateAfsExtractEntryToFile(IntPtr afs, int id, string output_folderpath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr DelegateAfsExtractEntryToBuffer(IntPtr afs, int id);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DelegateAfsFreeBuffer(IntPtr buffer);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate int DelegateAfsExtractFull(IntPtr afs, string output_folderpath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DelegateAfsReplaceEntry(IntPtr afs, int id, IntPtr data, int data_size);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DelegateAfsReplaceEntriesFromFiles(
            IntPtr afs,
            [In] int[] entries,
            [In, MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)]
            string[] filepaths,
            int amount_entries
        );

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate int DelegateAfsRenameEntry(IntPtr afs, int id, string new_name, bool permanent);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate AfsEntryInfo DelegateAfsGetEntryinfo(IntPtr afs, int id);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate AfsEntryMetadata DelegateAfsGetEntryMetadata(IntPtr afs, int id);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DelegateAfsSetEntryMetadata(IntPtr afs, int id, AfsEntryMetadata new_meta, bool permanent);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate Timestamp DelegateAfsGetLastModifiedDate(IntPtr afs, int id);

        // Function instances
        public static readonly DelegateAfsOpen afs_open;
        public static readonly DelegateAfsFree afs_free;
        public static readonly DelegateAfsGetEntrycount afs_getEntrycount;
        public static readonly DelegateAfsExtractEntryToFile afs_extractEntryToFile;
        public static readonly DelegateAfsExtractEntryToBuffer afs_extractEntryToBuffer;
        public static readonly DelegateAfsFreeBuffer afs_freeBuffer;
        public static readonly DelegateAfsExtractFull afs_extractFull;
        public static readonly DelegateAfsReplaceEntry afs_replaceEntry;
        public static readonly DelegateAfsReplaceEntriesFromFiles afs_replaceEntriesFromFiles;
        public static readonly DelegateAfsRenameEntry afs_renameEntry;
        public static readonly DelegateAfsGetEntryinfo afs_getEntryinfo;
        public static readonly DelegateAfsGetEntryMetadata afs_getEntryMetadata;
        public static readonly DelegateAfsSetEntryMetadata afs_setEntryMetadata;
        public static readonly DelegateAfsGetLastModifiedDate afs_getLastModifiedDate;

        // Handle to the native library
        private static readonly IntPtr nativeHandle;

        // Helper to map function name to delegate
        private static T GetDelegate<T>(string name) where T : Delegate
        {
            IntPtr ptr = NativeLibrary.GetExport(nativeHandle, name);
            return Marshal.GetDelegateForFunctionPointer<T>(ptr);
        }

        static AfsNative()
        {
            string libPath;
            string folder = AppContext.BaseDirectory;
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                libPath = "Afster.dll";
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
                libPath = "libAfster.so";
            else
                throw new PlatformNotSupportedException();

            libPath = Path.Combine(folder, libPath);   

            // Load the native library
            nativeHandle = NativeLibrary.Load(libPath);

            afs_open = GetDelegate<DelegateAfsOpen>("afs_open");
            afs_free = GetDelegate<DelegateAfsFree>("afs_free");
            afs_getEntrycount = GetDelegate<DelegateAfsGetEntrycount>("afs_getEntrycount");
            afs_extractEntryToFile = GetDelegate<DelegateAfsExtractEntryToFile>("afs_extractEntryToFile");
            afs_extractEntryToBuffer = GetDelegate<DelegateAfsExtractEntryToBuffer>("afs_extractEntryToBuffer");
            afs_freeBuffer = GetDelegate<DelegateAfsFreeBuffer>("afs_freeBuffer");
            afs_extractFull = GetDelegate<DelegateAfsExtractFull>("afs_extractFull");
            afs_replaceEntry = GetDelegate<DelegateAfsReplaceEntry>("afs_replaceEntry");
            afs_replaceEntriesFromFiles = GetDelegate<DelegateAfsReplaceEntriesFromFiles>("afs_replaceEntriesFromFiles");
            afs_renameEntry = GetDelegate<DelegateAfsRenameEntry>("afs_renameEntry");
            afs_getEntryinfo = GetDelegate<DelegateAfsGetEntryinfo>("afs_getEntryinfo");
            afs_getEntryMetadata = GetDelegate<DelegateAfsGetEntryMetadata>("afs_getEntryMetadata");
            afs_setEntryMetadata = GetDelegate<DelegateAfsSetEntryMetadata>("afs_setEntryMetadata");
            afs_getLastModifiedDate = GetDelegate<DelegateAfsGetLastModifiedDate>("afs_getLastModifiedDate");
        }
    }

    public sealed class Afs : IDisposable
    {
        public const int AFS_RESERVEDSPACEBUFFER = 2048;

        internal IntPtr _handle;
        private bool _disposed;
        private bool _opened;

        public bool Permanent;

        public string Path { get; private set; }

        public uint EntryCount { get; private set; }

        public Afs(string filepath)
        {
            _handle = AfsNative.afs_open(filepath);
            if (_handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to open AFS: {filepath}");
            }
            EntryCount = AfsNative.afs_getEntrycount(_handle);
            Path = filepath;
            _opened = true;
        }

        public AfsEntryInfo GetEntryInfo(int index)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            return AfsNative.afs_getEntryinfo(_handle, index);
        }

        public byte[] ExtractEntryToBuffer(int index)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            IntPtr bufferPtr = AfsNative.afs_extractEntryToBuffer(_handle, index);
            if (bufferPtr == IntPtr.Zero)
                throw new Exception("Failed to extract entry.");

            AfsEntryMetadata meta = GetEntryMetadata(index);
            byte[] buffer = new byte[meta.FileSize];
            Marshal.Copy(bufferPtr, buffer, 0, buffer.Length);
            AfsNative.afs_freeBuffer(bufferPtr);
            return buffer;
        }

        public string ExtractEntryToFile(int index, string outputFolder)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (!Directory.Exists(outputFolder))
            {
                throw new Exception($"Can't extract to {outputFolder} - Folder doesn't exist.");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            IntPtr afsreturn = AfsNative.afs_extractEntryToFile(_handle, index, outputFolder);
            if (afsreturn == IntPtr.Zero)
            {
                throw new Exception("Failed to extract entry to file.");
            }
            try
            {
                return Marshal.PtrToStringAnsi(afsreturn);
            }
            finally
            {
                AfsNative.afs_freeBuffer(afsreturn);
            }
        }

        public void ExtractFull(string outputFolder)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (!Directory.Exists(outputFolder))
            {
                throw new Exception($"Can't extract to {outputFolder} - Folder doesn't exist.");
            }
            int afsreturn = AfsNative.afs_extractFull(_handle, outputFolder);
            if (afsreturn == 1)
            {
                throw new Exception("AFS is invalid. (afs_extractFull returned 1)");
            }
        }

        public int ReplaceEntry(int index, byte[] data)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (data == null || data.Length <= 0)
            {
                throw new Exception("Data is invalid (null or null size)");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }

            GCHandle gch = GCHandle.Alloc(data, GCHandleType.Pinned);
            try
            {
                IntPtr ptr = gch.AddrOfPinnedObject();
                return AfsNative.afs_replaceEntry(_handle, index, ptr, data.Length);
            }
            finally
            {
                gch.Free();
            }
        }

        public int ReplaceEntriesFromFiles(int[] index_entries, string[] filepaths)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index_entries.Length != filepaths.Length)
            {
                throw new Exception("Array Length mismatch between entry IDs and Filepaths.");
            }
            int amount = index_entries.Length;

            return AfsNative.afs_replaceEntriesFromFiles(_handle, index_entries, filepaths, amount);
        }

        public int RenameEntry(int index, string new_name)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            if (string.IsNullOrWhiteSpace(new_name))
            {
                throw new Exception($"Invalid Name.");
            }
            return AfsNative.afs_renameEntry(_handle, index, new_name, Permanent);
        }

        public AfsEntryMetadata GetEntryMetadata(int index)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            return AfsNative.afs_getEntryMetadata(_handle, index);
        }

        public int SetEntryMetadata(int index, AfsEntryMetadata newMetadata)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            return AfsNative.afs_setEntryMetadata(_handle, index, newMetadata, Permanent);
        }

        public Timestamp GetLastModifiedDate(int index)
        {
            if (!_opened || _disposed)
            {
                throw new Exception("Invalid AFS");
            }
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            return AfsNative.afs_getLastModifiedDate(_handle, index);
        }

        public void Dispose()
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    AfsNative.afs_free(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
                GC.SuppressFinalize(this);
            }
        }
    }
}