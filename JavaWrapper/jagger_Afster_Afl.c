#include "jagger_Afster_Afl.h"
#include "../afl.h"

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_open
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jagger_Afster_Afl_afl_1open
  (JNIEnv * env, jobject caller, jstring aflPath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, aflPath, NULL);
    jlong handle = (jlong)afl_open(path);
    jni->ReleaseStringUTFChars(env, aflPath, path);
    return handle;
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_create
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jagger_Afster_Afl_afl_1create
  (JNIEnv * env, jobject caller, jlong afs, jstring filepath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, filepath, NULL);
    jlong handle = (jlong)afl_create((Afs*)afs, path);
    jni->ReleaseStringUTFChars(env, filepath, path);
    return handle;
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_new
 * Signature: (ILjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jagger_Afster_Afl_afl_1new
  (JNIEnv * env, jobject caller, jint entries, jstring filepath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, filepath, NULL);
    if(entries > 0xFFFF) {
      jclass exClass = jni->FindClass(env, "java/lang/IllegalArgumentException");
      jni->ThrowNew(env, exClass, "AFL Archives can only have a maximum of 65535 (0xFFFF) entries.");
      return 0;
    }
    jlong handle = (jlong)afl_new(entries, path);
    jni->ReleaseStringUTFChars(env, filepath, path);
    return handle;
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_getName
 * Signature: (JI)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jagger_Afster_Afl_afl_1getName
  (JNIEnv * env, jobject caller, jlong afl, jint id) {
    JNIEnv jni = *env;
    jstring name = jni->NewStringUTF(env, afl_getName((Afl*)afl, id));
    return name;
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_getEntrycount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afl_afl_1getEntrycount
  (JNIEnv * env, jobject caller, jlong afl) {
    JNIEnv jni = *env;
    return afl_getEntrycount((Afl*)afl);
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_rename
 * Signature: (JILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afl_afl_1rename
  (JNIEnv * env, jobject caller, jlong afl, jint id, jstring newName) {
    JNIEnv jni = *env;
    char* name = jni->GetStringUTFChars(env, newName, NULL);
    jint out = afl_rename((Afl*)afl, id, name);
    jni->ReleaseStringUTFChars(env, newName, name);
    return out;
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jagger_Afster_Afl_afl_1free
  (JNIEnv * env, jobject caller, jlong afl) {
    JNIEnv jni = *env;
    afl_free((Afl*)afl);
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_save
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afl_afl_1save
  (JNIEnv * env, jobject caller, jlong afl) {
    JNIEnv jni = *env;
    return afl_save((Afl*)afl);
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_saveNew
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afl_afl_1saveNew
  (JNIEnv * env, jobject caller, jlong afl, jstring filepath) {
    JNIEnv jni = *env;
    char* path = jni->GetStringUTFChars(env, filepath, NULL);
    jlong handle = (jlong)afl_saveNew((Afl*)afl, path);
    jni->ReleaseStringUTFChars(env, filepath, path);
    return handle;
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    afl_importAfl
 * Signature: (JJZ)I
 */
JNIEXPORT jint JNICALL Java_jagger_Afster_Afl_afl_1importAfl
  (JNIEnv * env, jobject caller, jlong afl, jlong afs, jboolean permanent) {
    JNIEnv jni = *env;
    return afl_importAfl((Afl*)afl, (Afs*) afs, permanent);
}

/*
 * Class:     jagger_Afster_Afl
 * Method:    jafl_getAllStrings
 * Signature: (J)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_jagger_Afster_Afl_jafl_1getAllStrings
  (JNIEnv * env, jobject caller, jlong afl) {
    JNIEnv jni = *env;
    jint entrycount = afl_getEntrycount((Afl*)afl);
    jbooleanArray entries = jni->NewObjectArray(env, entrycount, jni->FindClass(env, "java/lang/String"), NULL);

    for(int i=0;i<entrycount;i++) {
      jni->SetObjectArrayElement(env, entries, i, jni->NewStringUTF(env, afl_getName((Afl*)afl, i)));
    }

    return entries;
}
