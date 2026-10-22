#!/usr/bin/env python3
"""Patch glslang vulkan-sdk-1.3.268.0 so spv::Builder::createAccessChain computes
the result type from `base` (as in the glslang commit pinned by rexglue-sdk,
f4f1d8a352ca1908943aea2ad8c54b39b4879080) instead of from the builder's internal
accessChain state. The ReXGlue/Xenia SPIR-V translator calls createAccessChain
directly, so the newer behaviour aborts with
  getResultingAccessChainType(): assertion "accessChain.base != NoResult" failed
Idempotent: safe to run repeatedly on a cached checkout.
Usage: patch_glslang.py <path-to-glslang>/SPIRV/SpvBuilder.cpp
"""
import sys

OLD = """Id Builder::createAccessChain(StorageClass storageClass, Id base, const std::vector<Id>& offsets)
{
    // Figure out the final resulting type.
    Id typeId = getResultingAccessChainType();
    typeId = makePointer(storageClass, typeId);
"""
NEW = """Id Builder::createAccessChain(StorageClass storageClass, Id base, const std::vector<Id>& offsets)
{
    // Figure out the final resulting type. (pes2013: computed from `base`, as in
    // the glslang revision pinned by rexglue-sdk.)
    spv::Id typeId = getTypeId(base);
    assert(isPointerType(typeId) && offsets.size() > 0);
    typeId = getContainedTypeId(typeId);
    for (int i = 0; i < (int)offsets.size(); ++i) {
        if (isStructType(typeId)) {
            assert(isConstantScalar(offsets[i]));
            typeId = getContainedTypeId(typeId, getConstantScalar(offsets[i]));
        } else
            typeId = getContainedTypeId(typeId, offsets[i]);
    }
    typeId = makePointer(storageClass, typeId);
"""

def main(path):
    s = open(path, newline="").read()
    crlf = "\r\n" in s
    if crlf:
        s = s.replace("\r\n", "\n")
    if NEW in s:
        print("glslang createAccessChain: already patched")
        return 0
    if s.count(OLD) != 1:
        print("glslang createAccessChain: expected source not found (version changed?)", file=sys.stderr)
        return 1
    s = s.replace(OLD, NEW)
    if crlf:
        s = s.replace("\n", "\r\n")
    open(path, "w", newline="").write(s)
    print("glslang createAccessChain: patched")
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
