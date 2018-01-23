#include "kexecute.h"
#include "kmem.h"
#include "patchfinder64.h"

// offsets in vtable:
static uint32_t off_OSDictionary_SetObjectWithCharP = sizeof(void*) * 0x1F;
static uint32_t off_OSDictionary_GetObjectWithCharP = sizeof(void*) * 0x26;
static uint32_t off_OSDictionary_Merge              = sizeof(void*) * 0x23;

static uint32_t off_OSArray_Merge                   = sizeof(void*) * 0x1E;

// 1 on success, 0 on error
int OSDictionary_SetItem(uint64_t dict, const char *key, uint64_t val) {
	size_t len = strlen(key) + 1;

	uint64_t ks = kalloc(len);
	kwrite(ks, key, len);

	uint64_t vtab = rk64(dict);
	uint64_t f = rk64(vtab + off_OSDictionary_SetObjectWithCharP);

	int rv = (int) kexecute(f, dict, ks, val, 0, 0, 0, 0);

	kfree(ks, len);

	return rv;
}

// XXX it can return 0 in lower 32 bits but still be valid
// fix addr of returned value and check if rk64 gives ptr
// to vtable addr saved before

// address if exists, 0 if not
uint64_t _OSDictionary_GetItem(uint64_t dict, const char *key) {
	size_t len = strlen(key) + 1;

	uint64_t ks = kalloc(len);
	kwrite(ks, key, len);

	uint64_t vtab = rk64(dict);
	uint64_t f = rk64(vtab + off_OSDictionary_GetObjectWithCharP);

	int rv = (int) kexecute(f, dict, ks, 0, 0, 0, 0, 0);

	kfree(ks, len);

	return rv;
}

uint64_t OSDictionary_GetItem(uint64_t dict, const char *key) {
	uint64_t ret = _OSDictionary_GetItem(dict, key);
	
	if (ret != 0) {
		// XXX can it be not in zalloc?..
		ret = zm_fix_addr(ret);
	}

	return ret;
}

// 1 on success, 0 on error
int OSDictionary_Merge(uint64_t dict, uint64_t aDict) {
	uint64_t vtab = rk64(dict);
	uint64_t f = rk64(vtab + off_OSDictionary_Merge);

	return (int) kexecute(f, dict, aDict, 0, 0, 0, 0, 0);
}

// 1 on success, 0 on error
int OSArray_Merge(uint64_t array, uint64_t aArray) {
	uint64_t vtab = rk64(array);
	uint64_t f = rk64(vtab + off_OSArray_Merge);

	return (int) kexecute(f, array, aArray, 0, 0, 0, 0, 0);
}

// XXX error handling just for fun? :)
uint64_t _OSUnserializeXML(const char* buffer) {
	size_t len = strlen(buffer) + 1;

	uint64_t ks = kalloc(len);
	kwrite(ks, buffer, len);

	uint64_t errorptr = 0;

	uint64_t rv = kexecute(find_osunserializexml(), ks, errorptr, 0, 0, 0, 0, 0);
	kfree(ks, len);

	return rv;
}

uint64_t OSUnserializeXML(const char* buffer) {
	uint64_t ret = _OSUnserializeXML(buffer);
	
	if (ret != 0) {
		// XXX can it be not in zalloc?..
		ret = zm_fix_addr(ret);
	}

	return ret;
}
