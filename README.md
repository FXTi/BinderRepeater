# README

## Introduction

This is a repeater serving reproduce binder transaction from client side.

## Command line

```bash
./repeater service_name cmd flag base64(data) mDataSize mObjects mObjectsSize
```

* `service_name` should be identical to `adb shell service list` results, the same as in Service Manager.

* `base64(data) mDataSize mObjects mObjectsSize` these four arguments are the truly essential part of Parcel data.

After provide these arguments, a fake binder transaction will be sent to server.

## Implementation Detail

```c++
struct Parcel {
    status_t            mError;
    uint8_t*            mData;
    size_t              mDataSize;
    size_t              mDataCapacity;
    mutable size_t      mDataPos;
    binder_size_t*      mObjects;
    size_t              mObjectsSize;
    size_t              mObjectsCapacity;
    mutable size_t      mNextObjectHint;
    mutable bool        mObjectsSorted;

    mutable bool        mFdsKnown;
    mutable bool        mHasFds;
    bool                mAllowFds;

    release_func        mOwner;
    void*               mOwnerCookie;
};
```

Parcel has these private fields, but only some of them will be actually recorded and pass through the binder driver.

```c++
status_t IPCThreadState::writeTransactionData(int32_t cmd, uint32_t binderFlags,
    int32_t handle, uint32_t code, const Parcel& data, status_t* statusBuffer)
{
    binder_transaction_data tr;

    tr.target.ptr = 0; /* Don't pass uninitialized stack data to a remote process */
    tr.target.handle = handle;
    tr.code = code;
    tr.flags = binderFlags;
    tr.cookie = 0;
    tr.sender_pid = 0;
    tr.sender_euid = 0;

    const status_t err = data.errorCheck();
    if (err == NO_ERROR) {
        //Start Here
        tr.data_size = data.ipcDataSize();
        tr.data.ptr.buffer = data.ipcData();
        tr.offsets_size = data.ipcObjectsCount()*sizeof(binder_size_t);
        tr.data.ptr.offsets = data.ipcObjects();
        //End Here
    } else if (statusBuffer) {
        tr.flags |= TF_STATUS_CODE;
        *statusBuffer = err;
        tr.data_size = sizeof(status_t);
        tr.data.ptr.buffer = reinterpret_cast<uintptr_t>(statusBuffer);
        tr.offsets_size = 0;
        tr.data.ptr.offsets = 0;
    } else {
        return (mLastError = err);
    }

    mOut.writeInt32(cmd);
    mOut.write(&tr, sizeof(tr));

    return NO_ERROR;
}
```

The parts between comments is critical! After checking all four ipc*() functions, you' ll figure out the only four field passed out are `mData`, `mDataSize`, `mObjects`, and `mObjectsSize`.

The associated code which recover a Parcel from these fields can be found in binder_thread_read() in binder driver source code.

As a consequence, we will build a repeater which is quite general purpose to test server backed by binder. And here it is!  : )
