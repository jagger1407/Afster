package jagger.Afster;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;

public class Afl {
	static {
		loadLib("Afster");
        loadLib("AfsterNative");
    }

    private long _handle;
    private File _file;
    private int _entrycount;
    
    public final int AFL_NAMEBUFFERSIZE = 0x20;

    private static void loadLib(String libname) {
        if(System.getProperty("os.name").toLowerCase().contains("win")) {
            libname = "/" + libname + ".dll";
        } 
        else {
            libname = "/lib" + libname + ".so";
        }
    	try (InputStream in = Afl.class.getResourceAsStream(libname)) {
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
    
    private native long afl_open(String aflPath);
    private native long afl_create(long afs, String filepath);
    private native long afl_new(int entries, String filepath);
    private native String afl_getName(long afl, int id);
    private native int afl_getEntrycount(long afl);
    private native int afl_rename(long afl, int id, String new_name);
    private native void afl_free(long afl);
    private native int afl_save(long afl);
    private native int afl_saveNew(long afl, String filepath);
    private native int afl_importAfl(long afl, long afs, boolean permament);
    
    private native String[] jafl_getAllStrings(long afl);

    public Afl(String aflpath) {
        this._handle = afl_open(aflpath);
        this._file = new File(aflpath);
        this._entrycount = afl_getEntrycount(_handle);
    }
    
    public Afl(int amount_entries, String filepath) throws Exception {
    	if(amount_entries > 0xFFFF) {
    		throw new Exception("An AFL file can only have a maximum of 65535 entries.");
    	}
    	this._handle = afl_new(amount_entries, filepath);
    	this._file = new File(filepath);
    	this._entrycount = amount_entries;
    }
    
    public Afl(Afs afs, String filepath) {
    	this._handle = afl_create(afs._getHandle(), filepath);
    	this._file = new File(filepath);
    	this._entrycount = afl_getEntrycount(_handle);
    }

    public void free() {
        afl_free(_handle);
    }
    
    public long _getHandle() {
    	return _handle;
    }
    
    public String getName(int id) {
    	return afl_getName(_handle, id);
    }
    
    public int rename(int id, String new_name) {
    	return afl_rename(_handle, id, new_name);
    }
    
    public int getEntrycount() {
    	return _entrycount;
    }
    
    public int save() {
    	return afl_save(_handle);
    }
    
    public int saveNew(String filepath) {
    	return afl_saveNew(_handle, filepath);
    }
    
    public int importIntoAfs(Afs afs) {
    	return afl_importAfl(_handle, afs._getHandle(), afs.getPermanent());
    }
}
