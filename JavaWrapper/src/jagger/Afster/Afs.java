package jagger.Afster;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Afs {
	static {
		loadLib("Afster");
        loadLib("AfsterNative");
    }

    private long _handle;
    private File _file;
    private int _entrycount;
    private boolean _permanent;

    private static void loadLib(String libname) {
        if(System.getProperty("os.name").toLowerCase().contains("win")) {
            libname = "/" + libname + ".dll";
        } 
        else {
            libname = "/lib" + libname + ".so";
        }
    	try (InputStream in = Afs.class.getResourceAsStream(libname)) {
    		if(in == null) {
    			throw new UnsatisfiedLinkError(libname + " was not found.");
    		}
    		File tempLib = new File(System.getProperty("java.io.tmpdir"), libname.substring(1));
    		tempLib.deleteOnExit();
    		try (OutputStream out = new FileOutputStream(tempLib)) {
                byte[] buf = new byte[4096];
                int len;
                while ((len = in.read(buf)) > 0) {
                    out.write(buf, 0, len);
                }
            }
    		System.load(tempLib.getAbsolutePath());
    	}
    	catch (IOException ex) {
    		throw new RuntimeException(libname + " could not be loaded.");
    	}
    }
    
    private native long afs_open(String filePath);
    private native void afs_free(long afs);
    private native String afs_extractEntryToFile(long afs, int id, String outputFolderpath);
    private native byte[] afs_extractEntryToBuffer(long afs, int id);
    private native int afs_extractFull(long afs, String outputFolderpath);
    private native int afs_replaceEntry(long afs, int id, byte[] data);
    private native int afs_replaceEntriesFromFiles(long afs, int[] entries, String[] filepaths, int amountEntries);
    private native int afs_renameEntry(long afs, int id, String newName, boolean permanent);
    private native AfsEntryMetadata afs_getEntryMetadata(long afs, int id);
    private native int afs_setEntryMetadata(long afs, int id, AfsEntryMetadata new_meta, boolean permanent);
    
    private native int jafs_getEntrycount(long afs);
    private native int jafs_getEntryOffset(long afs, int id);
    private native int jafs_getEntryReservedSpace(long afs, int id);
    
    
    public Afs(String filePath) {
    	_file = new File(filePath);
    	_handle = afs_open(filePath);
    	_entrycount = jafs_getEntrycount(_handle);
    	_permanent = false;
    }
    
    public void free() {
    	afs_free(_handle);
    }
    public void close() {
    	free();
    }
    
    public long _getHandle() {
    	return _handle;
    }
    
    public int getEntryCount() {
    	return _entrycount;
    }
    
    public boolean getPermanent() { return _permanent; }
    public void setPermanent(boolean value) { _permanent = value; }
    
    public File extractEntryToFile(int id, String outputFolderpath) {
    	return new File(afs_extractEntryToFile(_handle, id, outputFolderpath));
    }
    
    public byte[] extractEntryToBuffer(int id) {
    	return afs_extractEntryToBuffer(_handle, id);
    }
    
    public int extractFull(String outputFolderpath) {
    	return afs_extractFull(_handle, outputFolderpath);
    }
    
    public int replaceEntry(int id, byte[] data) {
    	return afs_replaceEntry(_handle, id, data);
    }
    
    public int replaceEntriesFromFiles(int[] entryIds, String[] filepaths) {
    	return afs_replaceEntriesFromFiles(_handle, entryIds, filepaths, entryIds.length);
    }
    
    public int renameEntry(int id, String newName) {
    	return afs_renameEntry(_handle, id, newName, _permanent);
    }
    
    public AfsEntryMetadata getEntryMetadata(int id) {
    	return afs_getEntryMetadata(_handle, id);
    }
    
    public int setEntryMetadata(int id, AfsEntryMetadata meta) {
    	return afs_setEntryMetadata(_handle, id, meta, _permanent);
    }
    
    public int getEntryOffset(int id) {
    	return jafs_getEntryOffset(_handle, id);
    }
    
    public int getEntryReservedSpace(int id) {
    	return jafs_getEntryReservedSpace(_handle, id);
    }
}
