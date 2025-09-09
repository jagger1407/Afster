package jagger.Afster;

import java.time.LocalDateTime;

public class AfsEntryMetadata {
	public int size;
	public String entryName;
	public LocalDateTime lastModified;
	
	public AfsEntryMetadata() {
		size = 0;
		entryName = "";
		lastModified = null;
	}
	
	public AfsEntryMetadata(int size, String entryName, LocalDateTime lastModified) {
		this.size = size;
		this.entryName = entryName;
		this.lastModified = lastModified;
	}
	
	public AfsEntryMetadata(Afs afs, int id) {
		AfsEntryMetadata out = afs.getEntryMetadata(id);
		this.size = out.size;
		this.entryName = out.entryName;
		this.lastModified = out.lastModified;
	}
}
