#include "jagger_Afster_Afs.h"
#include "afs.h"

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_open
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jagger_Afster_Afs_afs_1open
  (JNIEnv * env, jobject caller, jstring filePath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, filePath, NULL);
    jlong handle = (jlong)afs_open(path);
    jni->ReleaseStringUTFChars(env, filePath, path);
    return handle;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jagger_Afster_Afs_afs_1free
  (JNIEnv * env, jobject caller, jlong afs) {
    JNIEnv jni = *env;
    afs_free((Afs*)afs);
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_extractEntryToFile
 * Signature: (JILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jagger_Afster_Afs_afs_1extractEntryToFile
  (JNIEnv * env, jobject caller, jlong afs, jint id, jstring outputFolderpath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, outputFolderpath, NULL);
    char* out = afs_extractEntryToFile((Afs*)afs, id, path);
    jni->ReleaseStringUTFChars(env, outputFolderpath, path);

    jstring outstr = jni->NewStringUTF(env, out);
    free(out);
    return outstr;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_extractEntryToBuffer
 * Signature: (JI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_jagger_Afster_Afs_afs_1extractEntryToBuffer
  (JNIEnv * env, jobject caller, jlong afs, jint id) {
    JNIEnv jni = *env;
    int size =  ((Afs*)afs)->header.entryinfo[id].size;
    u8* cBuf = afs_extractEntryToBuffer((Afs*)afs, id);
    jbyteArray jBuf = jni->NewByteArray(env, size);
    jni->SetByteArrayRegion(env, jBuf, 0, size, cBuf);
    free(cBuf);
    return jBuf;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_extractFull
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_afs_1extractFull
  (JNIEnv * env, jobject caller, jlong afs, jstring outputFolderpath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, outputFolderpath, NULL);
    jint out = afs_extractFull((Afs*)afs, path);
    jni->ReleaseStringUTFChars(env, outputFolderpath, path);
    return out;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_replaceEntry
 * Signature: (JI[B)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_afs_1replaceEntry
  (JNIEnv * env, jobject caller, jlong afs, jint id, jbyteArray data) {
    JNIEnv jni = *env;
    jboolean isCopy = false;
    u8* cData = jni->GetByteArrayElements(env, data, &isCopy);
    int newSize = jni->GetArrayLength(env, data);
    jint out = afs_replaceEntry((Afs*)afs, id, cData, newSize);
    if(isCopy) {
        free(cData);
    }
    return out;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_replaceEntriesFromFiles
 * Signature: (J[I[Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_afs_1replaceEntriesFromFiles
  (JNIEnv * env, jobject caller, jlong afs, jintArray entryIds, jobjectArray files, jint amountEntries) {
    JNIEnv jni = *env;
    int fcount = jni->GetArrayLength(env, files);
    char* filepaths[fcount];
    for(int i=0;i<fcount;i++) {
        jobject obj = jni->GetObjectArrayElement(env, files, i);
        char* file = jni->GetStringUTFChars(env, (jstring)obj, NULL);
        filepaths[i] = file;
    }
    int* ids = jni->GetIntArrayElements(env, entryIds, NULL);
    jint out = afs_replaceEntriesFromFiles((Afs*)afs, ids, filepaths, amountEntries);

    return out;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_renameEntry
 * Signature: (JILjava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_afs_1renameEntry
  (JNIEnv * env, jobject caller, jlong afs, jint id, jstring newName, jboolean permanent) {
    JNIEnv jni = *env;
    char* name = jni->GetStringUTFChars(env, newName, NULL);
    jint out = afs_renameEntry((Afs*)afs, id, name, permanent);
    return out;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_getEntryMetadata
 * Signature: (JI)Ljagger/Afster/AfsEntryMetadata;
 */
JNIEXPORT jobject JNICALL Java_jagger_Afster_Afs_afs_1getEntryMetadata
  (JNIEnv * env, jobject caller, jlong afs, jint id) {
    JNIEnv jni = *env;
    AfsEntryMetadata meta = afs_getEntryMetadata((Afs*)afs, id);
    jclass c = jni->FindClass(env, "jagger/Afster/AfsEntryMetadata");
    if(c == NULL) {
        return NULL;
    }
    jmethodID ctor = jni->GetMethodID(env, c, "<init>", "()V");
    if(ctor == NULL) {
        return NULL;
    }
    char* buf = (char*)malloc(AFSMETA_NAMEBUFFERSIZE + 1);
    memset(buf, 0x00, AFSMETA_NAMEBUFFERSIZE + 1);
    memcpy(buf, meta.filename, AFSMETA_NAMEBUFFERSIZE);
    jstring name = jni->NewStringUTF(env, buf);
    free(buf);
    
    jclass cDate = jni->FindClass(env, "java/time/LocalDateTime");
    jmethodID dateCtor = jni->GetStaticMethodID(env, cDate, "of", "(IIIIII)Ljava/time/LocalDateTime;");
    jobject jModified = jni->CallStaticObjectMethod(env, cDate, dateCtor, 
        meta.lastModified.year, meta.lastModified.month, meta.lastModified.day, meta.lastModified.hours, meta.lastModified.minutes, meta.lastModified.seconds);

    jobject jMeta = jni->NewObject(env, c, ctor);
    jfieldID fidSize = jni->GetFieldID(env, c, "size", "I");
    jfieldID fidName = jni->GetFieldID(env, c, "entryName", "Ljava/lang/String;");
    jfieldID fidModified = jni->GetFieldID(env, c, "lastModified", "Ljava/time/LocalDateTime;");
    jni->SetIntField(env, jMeta, fidSize, meta.filesize);
    jni->SetObjectField(env, jMeta, fidName, name);
    jni->SetObjectField(env, jMeta, fidModified, jModified);

    return jMeta;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    afs_setEntryMetadata
 * Signature: (JILjagger/Afster/AfsEntryMetadata;Z)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_afs_1setEntryMetadata
  (JNIEnv * env, jobject caller, jlong afs, jint id, jobject meta, jboolean permanent) {
    JNIEnv jni = *env;
    jclass cMeta = jni->FindClass(env, "jagger/Afster/AfsEntryMetadata");
    jclass cDate = jni->FindClass(env, "java/time/LocalDateTime");
    AfsEntryMetadata newMeta;
    jfieldID fidSize = jni->GetFieldID(env, cMeta, "size", "I");
    jfieldID fidName = jni->GetFieldID(env, cMeta, "entryName", "Ljava/lang/String;");
    jfieldID fidModified = jni->GetFieldID(env, cMeta, "lastModified", "Ljava/time/LocalDateTime;");
    int size = jni->GetIntField(env, meta, fidSize);
    jstring jName = (jstring)jni->GetObjectField(env, meta, fidName);
    char* cName = jni->GetStringUTFChars(env, jName, NULL);

    jmethodID midYear = jni->GetMethodID(env, cDate, "getYear", "()I");
    jmethodID midMonth = jni->GetMethodID(env, cDate, "getMonthValue", "()I");
    jmethodID midDay = jni->GetMethodID(env, cDate, "getDayOfMonth", "()I");
    jmethodID midHour = jni->GetMethodID(env, cDate, "getHour", "()I");
    jmethodID midMin = jni->GetMethodID(env, cDate, "getMinute", "()I");
    jmethodID midSec = jni->GetMethodID(env, cDate, "getSecond", "()I");

    newMeta.filesize = size;
    strncpy(newMeta.filename, cName, AFSMETA_NAMEBUFFERSIZE);
    newMeta.lastModified.year = jni->CallIntMethod(env, meta, midYear);
    newMeta.lastModified.month = jni->CallIntMethod(env, meta, midMonth);
    newMeta.lastModified.day = jni->CallIntMethod(env, meta, midDay);
    newMeta.lastModified.hours = jni->CallIntMethod(env, meta, midHour);
    newMeta.lastModified.minutes = jni->CallIntMethod(env, meta, midMin);
    newMeta.lastModified.seconds = jni->CallIntMethod(env, meta, midSec);

    jint out = afs_setEntryMetadata((Afs*)afs, id, newMeta, permanent);

    return out;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    jafs_getEntrycount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_jafs_1getEntrycount
  (JNIEnv * env, jobject caller, jlong afs) {
    JNIEnv jni = *env;
    jint out = ((Afs*)afs)->header.entrycount;
    return out;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    jafs_getEntryOffset
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_jafs_1getEntryOffset
  (JNIEnv * env, jobject caller, jlong afs, jint id) {
    return ((Afs*)afs)->header.entryinfo[id].offset;
}

/*
 * Class:     jagger_Afster_Afs
 * Method:    jafs_getEntryReservedSpace
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afs_jafs_1getEntryReservedSpace
  (JNIEnv * env, jobject caller, jlong afs, jint id) {
    return ((Afs*)afs)->header.entryinfo[id + 1].offset - ((Afs*)afs)->header.entryinfo[id].offset;
}