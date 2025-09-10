using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Afster
{
    internal static class AflNative
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate IntPtr DelegateAflOpen(string aflPath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate IntPtr DelegateAflCreate(IntPtr afs, string filepath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate IntPtr DelegateAflNew(ushort entries, string filepath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate string DelegateAflGetName(IntPtr afl, int id);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DelegateAflGetEntrycount(IntPtr afl);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate int DelegateAflRename(IntPtr afl, int id, string newName);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DelegateAflFree(IntPtr afl);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DelegateAflSave(IntPtr afl);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public delegate int DelegateAflSaveNew(IntPtr afl, string filepath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DelegateAflImport(IntPtr afl, IntPtr afs, bool permament);

        // Function instances
        public static readonly DelegateAflOpen afl_open;
        public static readonly DelegateAflCreate afl_create;
        public static readonly DelegateAflNew afl_new;
        public static readonly DelegateAflGetName afl_getName;
        public static readonly DelegateAflGetEntrycount afl_getEntrycount;
        public static readonly DelegateAflRename afl_rename;
        public static readonly DelegateAflFree afl_free;
        public static readonly DelegateAflSave afl_save;
        public static readonly DelegateAflSaveNew afl_saveNew;
        public static readonly DelegateAflImport afl_importAfl;

        // Handle to the native library
        private static readonly IntPtr nativeHandle;

        // Helper to map function name to delegate
        private static T GetDelegate<T>(string name) where T : Delegate
        {
            IntPtr ptr = NativeLibrary.GetExport(nativeHandle, name);
            return Marshal.GetDelegateForFunctionPointer<T>(ptr);
        }

        static AflNative()
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

            // Bind delegates to exported functions
            afl_open = GetDelegate<DelegateAflOpen>("afl_open");
            afl_create = GetDelegate<DelegateAflCreate>("afl_create");
            afl_new = GetDelegate<DelegateAflNew>("afl_new");
            afl_getName = GetDelegate<DelegateAflGetName>("afl_getName");
            afl_getEntrycount = GetDelegate<DelegateAflGetEntrycount>("afl_getEntrycount");
            afl_rename = GetDelegate<DelegateAflRename>("afl_rename");
            afl_free = GetDelegate<DelegateAflFree>("afl_free");
            afl_save = GetDelegate<DelegateAflSave>("afl_save");
            afl_saveNew = GetDelegate<DelegateAflSaveNew>("afl_saveNew");
            afl_importAfl = GetDelegate<DelegateAflImport>("afl_importAfl");
        }
    }

    public sealed class Afl : IDisposable
    {
        public const int AFL_NAMEBUFFERSIZE = AfsEntryMetadata.AFSMETA_NAMEBUFFERSIZE;
        internal IntPtr _handle;
        private bool _disposed;
        private bool _opened;

        public bool Permanent;

        public string Path { get; private set; }

        public int EntryCount { get; private set; }

        private Afl()
        {
            
        }

        public Afl(string filepath)
        {
            _handle = AflNative.afl_open(filepath);
            if (_handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to open AFL: {filepath}");
            }
            EntryCount = AflNative.afl_getEntrycount(_handle);
            Path = filepath;
            _opened = true;
        }

        public static Afl Create(Afs afs, string path)
        {
            Afl afl = new Afl();
            afl._handle = AflNative.afl_create(afs._handle, path);
            if (afl._handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to create AFL: {path}");
            }
            afl.EntryCount = AflNative.afl_getEntrycount(afl._handle);
            afl.Path = path;
            afl._opened = true;
            return afl;
        }

        public static Afl New(ushort entryCount, string path)
        {
            Afl afl = new Afl();
            afl._handle = AflNative.afl_new(entryCount, path);
            if (afl._handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to create AFL: {path}");
            }
            afl.EntryCount = entryCount;
            afl.Path = path;
            afl._opened = true;
            return afl;
        }

        public string GetName(int index)
        {
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFS range.");
            }
            return AflNative.afl_getName(_handle, index);
        }

        public int Rename(int index, string newName)
        {
            if (index < 0 || index >= EntryCount)
            {
                throw new Exception($"Index {index} outside of AFL range.");
            }
            return AflNative.afl_rename(_handle, index, newName);
        }

        public int Save()
        {
            return AflNative.afl_save(_handle);
        }

        public int SaveNew(string filepath)
        {
            return AflNative.afl_saveNew(_handle, filepath);
        }

        public void Dispose()
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    AflNative.afl_free(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
                GC.SuppressFinalize(this);
            }
        }
    }
}