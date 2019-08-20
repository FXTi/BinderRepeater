/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
   vi:ai:tabstop=8:shiftwidth=4:softtabstop=4:expandtab
*/

/*
 * Author: Gabriel Burca <gburca dash binder at ebixio dot com>
 *
 * Sample code for using binders in Android from C++
 *
 * The Demo service provides 3 operations: push(), alert(), add(). See
 * the IDemo class documentation to see what they do.
 *
 * Both the server and client code are included below.
 *
 * To view the log output:
 *      adb logcat -v time binder_demo:* *:S
 *
 * To run, create 2 adb shell sessions. In the first one run "binder" with no
 * arguments to start the service. In the second one run "binder N" where N is
 * an integer, to start a client that connects to the service and calls push(N),
 * alert(), and add(N, 5).
 */

#define LOG_TAG "binder_repeater"

/* For relevant code see:
    frameworks/native/{include,libs}/binder/{IInterface,Parcel}.{h,cpp}
    system/core/include/utils/{Errors,RefBase}.h
 */

#include <stdlib.h>
#include <string>

#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>

#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

using namespace android;


#define INFO(...) \
    do { \
        printf(__VA_ARGS__); \
        printf("\n"); \
        ALOGD(__VA_ARGS__); \
    } while(0)

void assert_fail(const char *file, int line, const char *func, const char *expr) {
    INFO("assertion failed at file %s, line %d, function %s:",
            file, line, func);
    INFO("%s", expr);
    abort();
}

#define ASSERT(e) \
    do { \
        if (!(e)) \
            assert_fail(__FILE__, __LINE__, __func__, #e); \
    } while(0)


// Where to print the parcel contents: aout, alog, aerr. alog doesn't seem to work.
#define PLOG aout

const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = 0; j < i; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

//Please keep sync with official implementation
struct Parcel_model {
    status_t            mError;
    uint8_t*            mData;  //useful
    size_t              mDataSize;  //useful
    size_t              mDataCapacity;
    mutable size_t      mDataPos;
    binder_size_t*               mObjects;   //useful
    size_t              mObjectsSize;   //useful
    size_t              mObjectsCapacity;
    mutable size_t      mNextObjectHint;
    mutable bool        mObjectsSorted;

    mutable bool        mFdsKnown;
    mutable bool        mHasFds;
    bool                mAllowFds;

    void*               mOwner;
    void*               mOwnerCookie;

    Parcel_model(char *data, size_t Dsize, uint64_t mobjects, size_t Osize): 
        mError(0), mData(0), mDataSize(Dsize),
        mDataCapacity(0), mDataPos(0), mObjects(0), mObjectsSize(Osize),
        mObjectsCapacity(0), mNextObjectHint(0), mObjectsSorted(0), mFdsKnown(0),
        mHasFds(0), mAllowFds(0), mOwner(0), mOwnerCookie(0) { 
            std::string res = base64_decode(std::string(data));
            mData = (uint8_t*)malloc(Dsize*sizeof(uint8_t));
            memcpy(mData, res.c_str(), Dsize);
            mObjects = (binder_size_t*)malloc(sizeof(binder_size_t));
            *mObjects = mobjects;
    }
};

int main(int argc, char **argv) {
    if (argc == 5) {
        sp<IServiceManager> sm = defaultServiceManager();
        ASSERT(sm != 0);
        sp<IBinder> binder = sm->getService(String16(argv[1]));
        // TODO: If the "Demo" service is not running, getService times out and binder == 0.
        ASSERT(binder != 0);
        Parcel reply;
        Parcel_model data(argv[4], atoll(argv[5]), atoll(argv[6]), atoll(argv[6]));
        binder->transact(atoll(argv[2]), *reinterpret_cast<Parcel*>(&data), &reply, atoll(argv[3]));
    } else {
        INFO("%s service_name cmd flag base64(data) mDataSize mObjects mObjectsSize", argv[0]);
    }

    return 0;
}

/*
    Single-threaded service, single-threaded client.
 */
